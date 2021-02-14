#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sodium.h>
#include <string.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640

double random_norm(){
    return (2 * (int)(randombytes_random() & 1) - 1) * (randombytes_random() / (double)(0xffffffff));
}

size_t fill_point_buffer(SDL_Point * point_buffer, char * point_tab){
    int ix = 0;
    for(int x = 0; x<SCREEN_WIDTH; x++){
        for(int y = 0; y<SCREEN_HEIGHT; y++){
            if(point_tab[x + y * SCREEN_WIDTH]){
                point_buffer[ix].x = x;
                point_buffer[ix].y = y;
                ix++;
            }
        }
    }
    return ix;
}

int create_char_texture(SDL_Renderer* renderer, char* filename, SDL_Texture** char_texture){
    SDL_Surface* char_surface = SDL_LoadBMP(filename);
    if(char_surface == NULL)
        return -1;

    *char_texture = SDL_CreateTextureFromSurface(renderer, char_surface);

    if(*char_texture == NULL)
        return -1;

    SDL_FreeSurface(char_surface);
    return 0;
}

void draw_char(SDL_Renderer* renderer, SDL_Texture* char_texture, int charidx, int pos, int off_x, int off_y, int isize){
    int w, h;
    SDL_QueryTexture(char_texture, NULL, NULL, &w, &h);
    SDL_Rect srcrect;
    srcrect.x = charidx * w / 10;
    srcrect.y = 0;
    srcrect.w = w / 10;
    srcrect.h = h;
    SDL_Rect dstrect;
    dstrect.x = off_x + pos * w / 10 / isize;
    dstrect.y = off_y;
    dstrect.w = w / 10 / isize;
    dstrect.h = h / isize;
    SDL_RenderCopy(renderer, char_texture, &srcrect, &dstrect);
}

void draw_chars(SDL_Renderer* renderer, SDL_Texture* char_texture, int* charidxs, int count, int off_x, int off_y, int isize){
    for(int i = 0; i<count; i++){
        if(charidxs[i]>=0)
            draw_char(renderer, char_texture, charidxs[i], i, off_x, off_y, isize);
    }
}

int main(){
    SDL_Window* window = NULL;
    SDL_Renderer * renderer = NULL;
    SDL_Surface* screenSurface = NULL;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    if (sodium_init() < 0) {
        fprintf(stderr, "libsodium couldn't be initialized\n");
        return 1;
    }

    window = SDL_CreateWindow("Monte carlo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (window == NULL) {
        fprintf(stderr, "could not create window: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer){
        fprintf(stderr, "could not create renderer: %s\n", SDL_GetError());
        return 1;
    }

    int64_t insideCount = 0;
    int64_t outsideCount = 0;

    char red_point_tab[SCREEN_WIDTH*SCREEN_HEIGHT];
    memset(red_point_tab, 0, SCREEN_WIDTH*SCREEN_HEIGHT);
    SDL_Point* red_point_buffer = malloc(sizeof(SDL_Point) * SCREEN_WIDTH * SCREEN_WIDTH);

    char blue_point_tab[SCREEN_WIDTH*SCREEN_HEIGHT];
    memset(blue_point_tab, 0, SCREEN_WIDTH*SCREEN_HEIGHT);
    SDL_Point* blue_point_buffer = malloc(sizeof(SDL_Point) * SCREEN_WIDTH * SCREEN_WIDTH);



    SDL_Texture * char_texture;
    if(create_char_texture(renderer, "digits.bmp", &char_texture) < 0){
        fprintf(stderr, "create_char_texture failed: %s\n", SDL_GetError());
        return 1;
    }

    int frame_counter = 0;
    int frame_counter_max = 5000;
    uint32_t last_ticks = SDL_GetTicks();

    while(1){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    SDL_DestroyWindow(window);
                    SDL_Quit();
                    return 0;
                    break;
            }
        }

        frame_counter = (frame_counter + 1) % frame_counter_max;
        if(!frame_counter){
            uint32_t new_ticks = SDL_GetTicks();
            printf("FPS: %lf\n", frame_counter_max/(double)((new_ticks-last_ticks))* 1000);
            last_ticks = new_ticks;
        }

        double x = random_norm();
        double y = random_norm();

        char inside = x*x+y*y <= 1;

        int xs = (x+1)/2 * SCREEN_WIDTH;
        int ys = (y+1)/2 * SCREEN_HEIGHT;

        insideCount += inside;
        outsideCount += !inside;

        double pi = 4 * insideCount / (double)(insideCount + outsideCount);

        if(inside){
            red_point_tab[xs + ys * SCREEN_WIDTH] = 1;
        }else{
            blue_point_tab[xs + ys * SCREEN_WIDTH] = 1;
        }

        int rn = fill_point_buffer(red_point_buffer, red_point_tab);
        int bn = fill_point_buffer(blue_point_buffer, blue_point_tab);

        //printf("%lf %ld\n", pi, insideCount + outsideCount);
        
        if(SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF) <0 ) fprintf(stderr, "background color %s\n", SDL_GetError());
        SDL_RenderClear(renderer);
        
        SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
        if(SDL_RenderDrawPoints(renderer, red_point_buffer, rn) < 0) fprintf(stderr, "draw points %s\n", SDL_GetError());

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);
        SDL_RenderDrawPoints(renderer, blue_point_buffer, bn);
    
        char pi_chars[30];
        sprintf(pi_chars, "%.15lf", pi);
        int pi_chars_indices[30];
        for(int i = 0; i<strlen(pi_chars); i++)
            pi_chars_indices[i] = pi_chars[i] - '0';
        draw_chars(renderer, char_texture, pi_chars_indices, strlen(pi_chars), 0, 0, 8);

        char iter_chars[30];
        sprintf(iter_chars, "%ld", insideCount+outsideCount);
        int iter_chars_indices[30];
        for(int i = 0; i<strlen(iter_chars); i++)
            iter_chars_indices[i] = iter_chars[i] - '0';
        draw_chars(renderer, char_texture, iter_chars_indices, strlen(iter_chars), 0, 50, 8);

        SDL_RenderPresent(renderer);
        SDL_UpdateWindowSurface(window);
        //SDL_Delay(1);
    }
}