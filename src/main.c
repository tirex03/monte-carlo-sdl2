#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <sodium.h>
#include <string.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640

double random_norm(){
    return (2 * (int)(randombytes_random() & 1) - 1) * (randombytes_random() / (double)(0xffffffff));
}

int create_font_texture(SDL_Renderer* renderer, char* filename, SDL_Texture** font_texture){
    SDL_Surface* char_surface = SDL_LoadBMP(filename);
    if(char_surface == NULL)
        return -1;

    *font_texture = SDL_CreateTextureFromSurface(renderer, char_surface);

    if(*font_texture == NULL)
        return -1;

    SDL_FreeSurface(char_surface);
    return 0;
}

void print_text(SDL_Renderer* renderer, SDL_Texture* font_texture, size_t font_size, SDL_Texture* canvas, int* font_map, char* text){
    int char_tex_w, char_tex_h;
    SDL_QueryTexture(font_texture, NULL, NULL, &char_tex_w, &char_tex_h);
    int char_w = char_tex_w / font_size;
    int char_h = char_tex_h;

    int canvas_w, canvas_h;
    SDL_QueryTexture(canvas, NULL, NULL, &canvas_w, &canvas_h);

    int isize = char_tex_h / canvas_h;

    SDL_Rect fontrect;
    SDL_Rect canvrect;

    fontrect.w = char_w;
    fontrect.h = char_h;
    fontrect.y = 0;

    canvrect.w = char_w / isize;
    canvrect.h = char_h / isize;
    canvrect.y = 0;

    SDL_SetRenderTarget(renderer, canvas);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
    SDL_RenderClear(renderer);
    for(int i = 0; i<strlen(text); i++){
        fontrect.x = font_map[text[i]] * char_w;
        canvrect.x = i * char_w / isize;
        SDL_RenderCopy(renderer, font_texture, &fontrect, &canvrect);
    }
    SDL_SetRenderTarget(renderer, NULL);
}

void draw_pixel(SDL_Renderer* renderer, SDL_Texture* canvas, SDL_Color* color, int x, int y){
    SDL_SetRenderTarget(renderer, canvas);
    SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
    SDL_RenderDrawPoint(renderer, x, y);
    SDL_SetRenderTarget(renderer, NULL);
}

int main(){
    SDL_Window* window = NULL;
    SDL_Renderer * renderer = NULL;
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

    SDL_Color color1;
    color1.r = 0xFF;
    color1.g = 0x00;
    color1.b = 0xFF;
    color1.a = 0xFF;

    SDL_Color color2;
    color2.r = 0x00;
    color2.g = 0xFF;
    color2.b = 0xFF;
    color2.a = 0xFF;

    SDL_Texture* canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_SetRenderTarget(renderer, canvas);
    SDL_RenderFillRect(renderer, NULL);
    SDL_SetRenderTarget(renderer, NULL);

    SDL_Texture* pi_text_canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, 600, 40);
    SDL_Texture* iter_text_canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, 600, 40);
    SDL_Rect pi_text_rect;
    pi_text_rect.x = 0;
    pi_text_rect.y = 0;
    SDL_QueryTexture(pi_text_canvas, NULL, NULL, &(pi_text_rect.w), &(pi_text_rect.h));

    SDL_Rect iter_text_rect;
    iter_text_rect.x = 0;
    iter_text_rect.y = 60;
    SDL_QueryTexture(iter_text_canvas, NULL, NULL, &(iter_text_rect.w), &(iter_text_rect.h));

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(pi_text_canvas, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(iter_text_canvas, SDL_BLENDMODE_BLEND);

    int64_t insideCount = 0;
    int64_t outsideCount = 0;


    char font_filename[1000];
    strcpy(font_filename, FONTDIR);
    strcat(font_filename, "/digits.bmp");

    SDL_Texture * font_texture;
    if(create_font_texture(renderer, font_filename, &font_texture) < 0){
        fprintf(stderr, "create_font_texture failed: %s\n", SDL_GetError());
        return 1;
    }

    int fontmap[255];
    fontmap['0'] = 0;
    fontmap['1'] = 1;
    fontmap['2'] = 2;
    fontmap['3'] = 3;
    fontmap['4'] = 4;
    fontmap['5'] = 5;
    fontmap['6'] = 6;
    fontmap['7'] = 7;
    fontmap['8'] = 8;
    fontmap['9'] = 9;
    fontmap['.'] = 10;

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
        
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);

        if(inside){
            draw_pixel(renderer, canvas, &color1, xs, ys);
        }else{
            draw_pixel(renderer, canvas, &color2, xs, ys);
        }

        SDL_RenderCopy(renderer, canvas, NULL, NULL);
    
        char pi_chars[30];
        char iter_chars[30];
        sprintf(pi_chars, "%.15lf", pi);
        sprintf(iter_chars, "%ld", insideCount+outsideCount);

        if(frame_counter % 200 == 0)
            print_text(renderer, font_texture, 11, pi_text_canvas, fontmap, pi_chars);
        SDL_RenderCopy(renderer, pi_text_canvas, NULL, &pi_text_rect);

        if(frame_counter % 200 == 0)
            print_text(renderer, font_texture, 11, iter_text_canvas, fontmap, iter_chars);
        SDL_RenderCopy(renderer, iter_text_canvas, NULL, &iter_text_rect);

        SDL_RenderPresent(renderer);
        SDL_UpdateWindowSurface(window);
        //SDL_Delay(1);
    }
}