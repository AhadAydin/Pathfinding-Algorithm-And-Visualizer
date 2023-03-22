

#include<iostream>
#include<string>
#include<SDL.h>
#include<vector>
#include<list>
#include<algorithm>

#undef main

enum class TileType {
	ground, wall, correctPath, visitedPath, start, finish
};
struct Tile {
	// x and y coordinates
	double x, y;
	// f cost = g cost + h cost
    int f_cost, g_cost, h_cost;
	TileType type;

    // pointer that points previous tile 
    Tile* cameFromTile;

    bool operator==(Tile const& other) {
        return(this->x == other.x && this->y == other.y);
    }
};
bool Compare(const Tile& first,const Tile& second) {
    if (first.f_cost == second.f_cost) {
        return first.h_cost < second.h_cost;
    }
    return first.f_cost < second.f_cost;
}

struct Color
{
	int r, g, b;
};

// Variables
int gridSize = 20;
int pixelRenderSize = 30;
bool s_input = false;
bool f_input = false;
Tile* startTile;
Tile* finishTile;
Color BLACK = { 0,0,0 }, WHITE = { 255,255,255 }, LIGHT_GRAY = { 200,200,200 }, MEDIUM_GRAY = {150,150,150}
, BLUE = { 0,0,255 }, RED = { 255,80,80 }, YELLOW = { 255,255,80 }, GREEN = { 80,255,80 };

#pragma region FUNCTIONS
void DrawScreen(std::vector<std::vector<Tile>>& grid, SDL_Renderer* renderer);
void HandleMouseClick(std::vector<std::vector<Tile>>& grid, int mouseX, int mouseY);
void UpdateScreen(std::vector<std::vector<Tile>>& grid, SDL_Renderer* renderer);
std::vector<Tile> FindPath(std::vector<std::vector<Tile>>& grid, Tile& start, Tile& finish, SDL_Renderer* renderer);
int CalculateDistanceCost(const Tile& a, const Tile& b);
std::vector<Tile> GetNeighborList(std::vector<std::vector<Tile>>& grid, Tile& currentTile);
std::vector<Tile> CalculatePath(Tile& finishTile);
#pragma endregion


#pragma region  INFO TEXT
const std::string INFO_TEXT = "A* PAHTFINDING ALGORITHM\nMouse click : wall/ground\nS + mouse click : start point\nF + mouse click : finish point\nEnter : Solve";
                               
#pragma endregion

int main() {

#pragma region GRID INITIALIZING

	std::vector<std::vector<Tile>> grid;
	for (int i = 0; i < gridSize; i++) {

		grid.push_back(std::vector<Tile>());
		for (int j = 0; j < gridSize; j++) {

            grid[i].push_back(Tile());

            grid[i][j].x = i; grid[i][j].y = j;

            if (i == 0 || i == gridSize - 1 || j == 0 || j == gridSize - 1) {// wall
                grid[i][j].type = TileType::wall;
            }
            else {// ground
                grid[i][j].type = TileType::ground;
            }
		}
	}
    grid[1][1].type = TileType::start; // initial start point
    startTile = &grid[1][1];
    grid[gridSize - (double)2][gridSize - (double)2].type = TileType::finish; // initial finish point
    finishTile = &grid[gridSize - (double)2][gridSize - (double)2];
#pragma endregion

	// Initialize renderer
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	int screenSize = gridSize * pixelRenderSize;
	SDL_CreateWindowAndRenderer(screenSize, screenSize, 0, &window, &renderer);
	SDL_RenderSetScale(renderer, pixelRenderSize, pixelRenderSize);
	SDL_Event event;

    bool running = true;
    bool enter_input = false;

    std::cout << INFO_TEXT << std::endl;
    while (running) {

        while (SDL_PollEvent(&event))
        {

            if (event.type == SDL_QUIT) { // user pressed to quit button
                running = false;
            }
            else if (event.type == SDL_KEYDOWN) {

                // detect enter input start 
                if (event.key.keysym.sym == SDLK_RETURN && enter_input == false) {

                    std::cout << "Enter has been pressed\n";
                    enter_input = true;

                    std::vector<Tile> solutionPath = FindPath(grid, *startTile, *finishTile, renderer);

                }
                if (event.key.keysym.sym == SDLK_s)
                    s_input = true;
                if (event.key.keysym.sym == SDLK_f)
                    f_input = true;
            }
            else if (event.type == SDL_KEYUP) {

                // detect enter input released
                if (event.key.keysym.sym == SDLK_RETURN) {
                    enter_input = false;
                }
                if (event.key.keysym.sym == SDLK_s)
                    s_input = false;
                if (event.key.keysym.sym == SDLK_f)
                    f_input = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {

                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                HandleMouseClick(grid, mouseX, mouseY);
            }

        }

        SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, 255);
        SDL_RenderClear(renderer);

        DrawScreen(grid, renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(5);
    }


	return 0;
}


void DrawScreen(std::vector<std::vector<Tile>>& grid, SDL_Renderer* renderer) {

    for (int i = 0; i < grid.size(); i++) {

        for (int j = 0; j < grid[i].size(); j++) {

            if (grid[i][j].type == TileType::ground) { // it's a ground tile

                if ((i + j) % 2 == 0) {
                    SDL_SetRenderDrawColor(renderer, MEDIUM_GRAY.r, MEDIUM_GRAY.g, MEDIUM_GRAY.b, 255);
                }
                else {
                    SDL_SetRenderDrawColor(renderer, LIGHT_GRAY.r, LIGHT_GRAY.g, LIGHT_GRAY.b, 255);
                }
            }
            else if (grid[i][j].type == TileType::wall) { // it's a wall tile
                SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, 255);
            }
            else if (grid[i][j].type == TileType::correctPath) { // it's a path tile
                SDL_SetRenderDrawColor(renderer, BLUE.r, BLUE.g, BLUE.b, 255);
            }
            else if (grid[i][j].type == TileType::finish) {
                SDL_SetRenderDrawColor(renderer, GREEN.r, GREEN.g, GREEN.b, 255);
            }
            else if (grid[i][j].type == TileType::start) {
                SDL_SetRenderDrawColor(renderer, YELLOW.r, YELLOW.g, YELLOW.b, 255);
            }
            else { // it's a wrong path tile
                SDL_SetRenderDrawColor(renderer, RED.r, RED.g, RED.b, 255);
            }

            // paint the tile
            SDL_RenderDrawPoint(renderer, i, j);
        }
    }

}

void HandleMouseClick(std::vector<std::vector<Tile>>& grid, int mouseX, int mouseY) {

    // found the tile based on pixel coordinates
    int x = mouseX / pixelRenderSize;
    int y = mouseY / pixelRenderSize;

    if (x == 0 || x == gridSize - 1 || y == 0 || y == gridSize - 1) {
        // Cannot change border tiles
        std::cout << "Border tiles must be wall only...\n";
    }
    else if (grid[x][y].type == TileType::start || grid[x][y].type == TileType::finish) {
        // Cannot change start or finish tile
        std::cout << "Cannot change a start or finish tile, but you can change the position of them...\n";
    }
    else {

        if (s_input) {
            startTile->type = TileType::ground;
            startTile = &grid[x][y];
            grid[x][y].type = TileType::start;
        }
        else if (f_input) {
            finishTile->type = TileType::ground;
            finishTile = &grid[x][y];
            grid[x][y].type = TileType::finish;
        }
        else {
            // turn walls into grounds and others to wall
            if (grid[x][y].type == TileType::wall) {
                grid[x][y].type = TileType::ground;
            }
            else {
                grid[x][y].type = TileType::wall;
            }

        }
    }

}

void UpdateScreen(std::vector<std::vector<Tile>>& grid, SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, 255);
    SDL_RenderClear(renderer);

    DrawScreen(grid, renderer);

    SDL_RenderPresent(renderer);
    SDL_Delay(40);
}

std::vector<Tile> FindPath(std::vector<std::vector<Tile>>& grid, Tile& start, Tile& finish, SDL_Renderer* renderer) {

    std::vector<Tile> openList;
    openList.push_back(start);
    std::vector<Tile> closedList; 

    // reser all tiles
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            if (grid[i][j].type == TileType::visitedPath || grid[i][j].type == TileType::correctPath)
                grid[i][j].type = TileType::ground;

            grid[i][j].g_cost = -1;
            grid[i][j].f_cost = -1;
            grid[i][j].cameFromTile = nullptr;
        }
    }

    start.g_cost = 0;
    start.h_cost = CalculateDistanceCost(start, finish);
    start.f_cost = start.g_cost + finish.h_cost;

    while (openList.size() > 0)
    {
        // looking to first tile, it has the lowest f cost 
        Tile currentTile = openList.front();

        if (openList.front() == finish) {
            // Found finish
            return CalculatePath(finish);
        }

        closedList.push_back(openList.front());
        openList.erase(openList.begin());

        std::vector<Tile> neighborList = GetNeighborList(grid, currentTile);
        for (auto neighborItr : neighborList) {

            std::vector<Tile>::iterator closedListFoundItr = std::find(closedList.begin(), closedList.end(), neighborItr);
            if (closedListFoundItr != closedList.end()) {
                continue;
            }

            int tentativeGCost = currentTile.g_cost + CalculateDistanceCost(currentTile, neighborItr);
            if (neighborItr.g_cost == -1) {

                if (grid[neighborItr.x][neighborItr.y].type != TileType::start && grid[neighborItr.x][neighborItr.y].type != TileType::finish)
                    grid[neighborItr.x][neighborItr.y].type = TileType::visitedPath;
                UpdateScreen(grid, renderer);

                grid[neighborItr.x][neighborItr.y].cameFromTile = &grid[currentTile.x][currentTile.y];
                neighborItr.g_cost = tentativeGCost;
                neighborItr.h_cost = CalculateDistanceCost(neighborItr, finish);
                neighborItr.f_cost = neighborItr.g_cost + neighborItr.h_cost;
            }

            std::vector<Tile>::iterator openListFoundItr = std::find(openList.begin(), openList.end(), neighborItr);
            if (openListFoundItr == openList.end()) {
                openList.push_back(neighborItr);
                std::sort(openList.begin(), openList.end(), Compare);
            }
        }
    }

    // End of open list
    std::cout << "couldnt find a way...\n ";
}

int CalculateDistanceCost(const Tile& a, const Tile& b){

    int x_distance = abs(a.x - b.x);
    int y_distance = abs(a.y - b.y);
    int remaining = abs(x_distance - y_distance);

    // diagonal cost : 14, straight cost : 10
    return 14 * std::min(x_distance, y_distance) + 10 * remaining;
}

std::vector<Tile> GetNeighborList(std::vector<std::vector<Tile>>& grid, Tile& currentTile) {

    std::vector<Tile> neighborList;

    if (currentTile.x - 1 >= 0) {
        //Left
        if (grid[currentTile.x - 1][currentTile.y].type != TileType::wall)
            neighborList.push_back(grid[currentTile.x - 1][currentTile.y]);
        // Left Up
        if (currentTile.y - 1 >= 0)
            if (grid[currentTile.x - 1][currentTile.y - 1].type != TileType::wall)
                neighborList.push_back(grid[currentTile.x - 1][currentTile.y - 1]);
        // Left Down
        if (currentTile.y + 1 < gridSize)
            if (grid[currentTile.x - 1][currentTile.y + 1].type != TileType::wall)
                neighborList.push_back(grid[currentTile.x - 1][currentTile.y + 1]);
    }
    if (currentTile.x + 1 < gridSize) {
        //Right
        if (grid[currentTile.x + 1][currentTile.y].type != TileType::wall)
            neighborList.push_back(grid[currentTile.x + 1][currentTile.y]);
        // Right Up
        if (currentTile.y + 1 >= 0)
            if (grid[currentTile.x + 1][currentTile.y - 1].type != TileType::wall)
                neighborList.push_back(grid[currentTile.x + 1][currentTile.y - 1]);
        // Right Down
        if (currentTile.y + 1 < gridSize)
            if (grid[currentTile.x + 1][currentTile.y + 1].type != TileType::wall)
                neighborList.push_back(grid[currentTile.x + 1][currentTile.y + 1]);
    }
    // Up
    if(currentTile.y - 1 >= 0)
        if (grid[currentTile.x][currentTile.y - 1].type != TileType::wall)
            neighborList.push_back(grid[currentTile.x][currentTile.y - 1]);
    // Down
    if (currentTile.y + 1 < gridSize)
        if (grid[currentTile.x][currentTile.y + 1].type != TileType::wall)
            neighborList.push_back(grid[currentTile.x][currentTile.y + 1]);

    return neighborList;

}

std::vector<Tile> CalculatePath(Tile& finishTile) {
    std::vector<Tile> path;
    path.push_back(finishTile);
    Tile* currentTile = &finishTile;
    while (currentTile->cameFromTile != NULL) {

        if (currentTile->type != TileType::start && currentTile->type != TileType::finish)
            currentTile->type = TileType::correctPath;

        path.insert(path.begin(), *currentTile->cameFromTile);
        currentTile = currentTile->cameFromTile;
    }

    return path;
}