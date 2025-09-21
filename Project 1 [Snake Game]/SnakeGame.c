#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <setjmp.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SNAKE_MAX_LENGTH 500
#define PIXEL_SIZE 20
#define MOVE_DELAY 120

typedef struct Entity_type{
    SDL_Rect Body;
    unsigned char Red;
    unsigned char Green;
    unsigned char Blue;
}Entity;

//Function Declarations
bool init();
void end();
bool makewindow();
void gameloop();
void move(const SDL_Scancode direction, int snake_size);
void Make_Snake(int snake_size, SDL_Scancode direction);
void Make_Apple();
bool is_snake_there(Entity entity[], int entity_size, int snake_size);

SDL_Window *gWindow = NULL;
SDL_Surface *gScreenSurface = NULL;
Entity Snake[SNAKE_MAX_LENGTH + 1]; //0 is for head. Rest for Body
Entity Apple;
jmp_buf jmp;

int main(int argc, char *argv[]) {
    //initialize
    if(!init())
       return -1;

    //create window
    if(!makewindow())
       return -2;

    gameloop();

    end();
    
    return 0;
}
bool makewindow() {
    gWindow = SDL_CreateWindow("Snake Game", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if(gWindow == NULL) {
        SDL_Log("Window Creation Failed! Cuz %s\n", SDL_GetError());
        return false;
    }

    //get draw surface
    gScreenSurface = SDL_GetWindowSurface(gWindow);

    //Make The window Sea Green
    SDL_FillSurfaceRect(gScreenSurface, NULL, SDL_MapSurfaceRGB(gScreenSurface, 0x3e, 0x8b, 0x57));
    SDL_UpdateWindowSurface(gWindow);

    return true;
}
void gameloop() {
    setjmp(jmp);
    //Main game loop
    bool quit = false;
    SDL_Event event;
    SDL_zero(event); //Ensure we are dealing with zeroed out memory
    int snake_len = 0;
    Uint64 last_move_time = 0;
    SDL_Scancode current_direction = 0;

    //Set everything up first
    Make_Snake(snake_len++, 0); //Passing 0: snake_len is zero it will not go to the else clause
    Make_Apple();
    while(!quit) {
        //Start by processing events
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_EVENT_QUIT) {
                quit = true;
            }
            else if(event.type == SDL_EVENT_KEY_DOWN) {
                current_direction = event.key.scancode;
            }
        }

        //Checking for collisions
        if(SDL_GetTicks() - last_move_time > MOVE_DELAY) {
            move(current_direction, snake_len);
            last_move_time += MOVE_DELAY;
        }
        //Passing SnakeHead to check its collision with itself
        if(is_snake_there(&Snake[0], 1, snake_len)) {
            for(;;) {
                //flushing keypress and mouse movement
                SDL_FlushEvent(SDL_EVENT_KEY_DOWN);
                SDL_FlushEvent(SDL_EVENT_MOUSE_MOTION);
                //Blocking until any keypress
                if(!SDL_WaitEvent(&event)) {
                    SDL_Log("Event waiting failed! Cuz %s\n", SDL_GetError());
                }
                if(event.type == SDL_EVENT_KEY_DOWN) {
                    longjmp(jmp, 1);
                }
            }
        }
        if(Apple.Body.x == Snake[0].Body.x && Apple.Body.y == Snake[0].Body.y) {
            Make_Snake(snake_len++, current_direction);
            Make_Apple();
            //Checking if Apple spawned where the snake is
            while(is_snake_there(&Apple, 1, snake_len)) {
                Make_Apple();
            }
        }

        //Clear Surface
        SDL_FillSurfaceRect(gScreenSurface, NULL, SDL_MapSurfaceRGB(gScreenSurface, 0x3e, 0x8b, 0x57));
        
        //Then perform updates
        for(int i = 0; i <= snake_len; i++) {
            SDL_FillSurfaceRect(gScreenSurface, &Snake[i].Body, SDL_MapSurfaceRGB(gScreenSurface, Snake[i].Red, Snake[i].Green, Snake[i].Blue)); //Updates Snake Location
        }
        SDL_FillSurfaceRect(gScreenSurface, &Apple.Body, SDL_MapSurfaceRGB(gScreenSurface, Apple.Red, Apple.Green, Apple.Blue));  //Updates Apple Location

        //Finally display the updated surface
        SDL_UpdateWindowSurface(gWindow);
    }
}
void move(const SDL_Scancode direction, int snake_size) {
    for(int i = snake_size; i >= 0; i--) {
        //First update the head's location
        if(direction == SDL_SCANCODE_W) {
            //Go Up
            if(i == 0)
                Snake[i].Body.y -= PIXEL_SIZE;
        }
        else if(direction == SDL_SCANCODE_S) {
            //Go Down
            if(i == 0)
                Snake[i].Body.y += PIXEL_SIZE;
        }
        else if(direction == SDL_SCANCODE_A) {
            //Go Left
            if(i == 0)
                Snake[i].Body.x -= PIXEL_SIZE;
        }
        else if(direction == SDL_SCANCODE_D) {
            //Go Right
            if(i == 0)
                Snake[i].Body.x += PIXEL_SIZE;
        }
        //Then update the rest of the body (From the tail)
        if(i != 0) {
            Snake[i].Body.x = Snake[i - 1].Body.x;
            Snake[i].Body.y = Snake[i - 1].Body.y;
        }
        //To Wrap Around, Handling Negative cases + Overflow
        Snake[i].Body.x = (Snake[i].Body.x + SCREEN_WIDTH) % SCREEN_WIDTH;
        Snake[i].Body.y = (Snake[i].Body.y + SCREEN_HEIGHT) % SCREEN_HEIGHT;
    }
}
void Make_Snake(int snake_size, SDL_Scancode direction) {
    Snake[snake_size].Body.h = PIXEL_SIZE;
    Snake[snake_size].Body.w = PIXEL_SIZE;
    if(snake_size == 0) {
        //Spawning head in the middle
        Snake[snake_size].Body.x = SCREEN_WIDTH/2;
        Snake[snake_size].Body.y = SCREEN_HEIGHT/2;
    }
    else {
        //Spawning the next of the body part before the tail
        if(direction == SDL_SCANCODE_W) {
            Snake[snake_size].Body.y = Snake[snake_size - 1].Body.y + PIXEL_SIZE;
        }
        else if(direction == SDL_SCANCODE_S) {
            Snake[snake_size].Body.y = Snake[snake_size - 1].Body.y - PIXEL_SIZE;
        }
        else if(direction == SDL_SCANCODE_A) {
            Snake[snake_size].Body.x = Snake[snake_size - 1].Body.x + PIXEL_SIZE;
        }
        else if(direction == SDL_SCANCODE_D) {
            Snake[snake_size].Body.x = Snake[snake_size - 1].Body.x - PIXEL_SIZE;
        }
    }
    Snake[snake_size].Red = 0x6F;
    Snake[snake_size].Green = 0x46;
    Snake[snake_size].Blue = 0x85;
}
void Make_Apple() {
    Apple.Body.h = PIXEL_SIZE;
    Apple.Body.w = PIXEL_SIZE;
    //Random location for Apple
    Apple.Body.x = PIXEL_SIZE * (SDL_rand(SCREEN_WIDTH)%PIXEL_SIZE);
    Apple.Body.y = PIXEL_SIZE * (SDL_rand(SCREEN_HEIGHT)%PIXEL_SIZE);

    Apple.Red = 0xDD;
    Apple.Green = 0x15;
    Apple.Blue = 0x33;
}
bool is_snake_there(Entity entity[], int entity_size, int snake_size) {
    for(int i = 0; i < entity_size; i++) {
        for(int j = 1; j < snake_size; j++) {
            if(entity[i].Body.x == Snake[j].Body.x && entity[i].Body.y == Snake[j].Body.y)
                return true;
        }
    }

    return false;
}

bool init() {
    bool success = SDL_Init(SDL_INIT_VIDEO);
    if(!success) {
        SDL_Log("Initialization failed! Cuz: %s\n", SDL_GetError());
    }

    return success;
}
void end() {
    //Destroy window
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    gScreenSurface = NULL;
    
    SDL_Quit();
}