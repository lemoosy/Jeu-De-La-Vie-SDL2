#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>


#define SIZE_X_MATRIX   100
#define SIZE_Y_MATRIX   100
#define SIZE_X_WINDOW   500
#define SIZE_Y_WINDOW   500
#define SIZE_X_CELL     SIZE_X_WINDOW / SIZE_X_MATRIX
#define SIZE_Y_CELL     SIZE_Y_WINDOW / SIZE_Y_MATRIX
#define TIME_REFRESH    10
#define STEP            30


struct Color { int R, G, B; };


int out_of_dimension(int x, int y)
{
    return x < 0 || x >= SIZE_X_MATRIX || y < 0 || y >= SIZE_Y_MATRIX;
}


int count_cells_around(int** matrix, int x, int y)
{
    int amount = 0;

    for (int j = -1; j < 2; j++)
        for (int i = -1; i < 2; i++)
            if (i != 0 || j != 0)
                if (!out_of_dimension(x + i, y + j))
                    amount += matrix[y + j][x + i] == STEP;

    return amount;
}


void switch_values(int* a, int* b)
{
    *a = *a + *b;
    *b = *a - *b;
    *a = *a - *b;
}


void copy_matrix(int** matrix1, int** matrix2)
{
    for (int y = 0; y < SIZE_Y_MATRIX; y++)
        for (int x = 0; x < SIZE_X_MATRIX; x++)
            switch_values(&matrix1[y][x], &matrix2[y][x]);
}


void fill_matrix(int** matrix, int value)
{
    for (int y = 0; y < SIZE_Y_MATRIX; y++)
        for (int x = 0; x < SIZE_X_MATRIX; x++)
            matrix[y][x] = value;
}


void fill_matrix_random(int** matrix)
{
    srand(time(NULL));

    for (int y = 0; y < SIZE_Y_MATRIX; y++)
        for (int x = 0; x < SIZE_X_MATRIX; x++)
            matrix[y][x] = (rand() % 2 == 1) * STEP;
}


void draw_square(SDL_Renderer* renderer, int x, int y, int w, int h, int R, int G, int B)
{
    // Création du carré
    SDL_Rect square = { x, y, w, h };

    // Remplie le carré
    SDL_SetRenderDrawColor(renderer, R, G, B, 255);

    // Dessine le carré
    SDL_RenderFillRect(renderer, &square);
}


void generate_colors(struct Color colors[])
{
    //	R		G		B
    //	255		0		0		ROUGE
    //	255		255		0		JAUNE
    //	0		255		0		VERT

    int x1 = STEP / 2;
    int x2 = STEP - x1;

    for (int i = 0; i < x1; i++)
    {
        colors[i].R = 255;
        colors[i].G = (255 / x1) * i;
        colors[i].B = 0;
    }

    for (int i = 0; i < x2; i++)
    {
        colors[x1 + i].R = 255 - (255 / (x2 - 1)) * i;
        colors[x1 + i].G = 255;
        colors[x1 + i].B = 0;
    }
}


void draw_matrix(SDL_Renderer* renderer, int** matrix, struct Color colors[])
{
    // Modifie la couleur du fond
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // Efface le contenu dans la fenêtre
    SDL_RenderClear(renderer);

    for (int y = 0; y < SIZE_Y_MATRIX; y++)
        for (int x = 0; x < SIZE_X_MATRIX; x++)
            if (matrix[y][x] != 0)
                draw_square
                (
                    renderer,
                    SIZE_X_CELL * x,
                    SIZE_Y_CELL * y,
                    SIZE_X_CELL,
                    SIZE_Y_CELL,
                    colors[matrix[y][x] - 1].R,
                    colors[matrix[y][x] - 1].G,
                    colors[matrix[y][x] - 1].B
                );

    // Actualise le rendu
    SDL_RenderPresent(renderer);
}


int main(int argc, char* argv[])
{
    // Initialisation de SDL

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", SDL_GetError());
        return 0;
    }

    // Initialisation de la fenêtre

    SDL_Window* window = SDL_CreateWindow("Jeu de la vie v6", SIZE_X_WINDOW, SDL_WINDOWPOS_CENTERED, SIZE_X_WINDOW, SIZE_Y_WINDOW, SDL_WINDOW_SHOWN);

    if (window == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    // Initialisation du rendu

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // Initialisation des matrices

    int** old_matrix = calloc(SIZE_Y_MATRIX, sizeof(int*));
    int** new_matrix = calloc(SIZE_Y_MATRIX, sizeof(int*));

    for (int y = 0; y < SIZE_Y_MATRIX; y++)
    {
        old_matrix[y] = calloc(SIZE_X_MATRIX, sizeof(int));
        new_matrix[y] = calloc(SIZE_X_MATRIX, sizeof(int));
    }

    fill_matrix_random(old_matrix);

    // Jeu

    SDL_Event events;
    struct Color colors[STEP];
    generate_colors(colors);

    while (1)
    {
        if (SDL_PollEvent(&events))
            if (events.type == SDL_QUIT)
                break;

        for (int y = 0; y < SIZE_Y_MATRIX; y++)
            for (int x = 0; x < SIZE_X_MATRIX; x++)
            {
                if (count_cells_around(old_matrix, x, y) == 3)
                    new_matrix[y][x] = STEP;

                if (count_cells_around(old_matrix, x, y) == 2)
                    new_matrix[y][x] = old_matrix[y][x] - (0 < old_matrix[y][x] && old_matrix[y][x] < STEP);

                if ((count_cells_around(old_matrix, x, y) < 2 || count_cells_around(old_matrix, x, y) > 3) && old_matrix[y][x] > 0)
                    new_matrix[y][x] = old_matrix[y][x] - 1;
            }

        copy_matrix(old_matrix, new_matrix);
        fill_matrix(new_matrix, 0);
        draw_matrix(renderer, old_matrix, colors);
        SDL_Delay(TIME_REFRESH);
    }

    // Libère la mémoire

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    // Fin

    return 0;
}


// Léonard Lemoosy | Jeu de la vie v6