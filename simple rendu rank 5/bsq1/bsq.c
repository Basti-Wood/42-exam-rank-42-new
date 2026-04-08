#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct s_map
{
    int rows;
    int cols;
    char empty;
    char obstacle;
    char full;
    char **grid;
} t_map;

// -------------------- FREE MAP --------------------
void free_map(t_map *map)
{
    if (!map || !map->grid)
        return;
    for (int i = 0; i < map->rows; i++)
        free(map->grid[i]);
    free(map->grid);
}

// -------------------- ERROR --------------------
int map_error(t_map *map)
{
    fprintf(stderr, "map error\n");
    free_map(map);
    return (0);
}

// -------------------- PARSE FIRST LINE --------------------
int parse_header(char *line, t_map *map)
{
    if (sscanf(line, "%d %c %c %c",
        &map->rows, &map->empty, &map->obstacle, &map->full) != 4)
        return (0);

    if (map->empty == map->obstacle ||
        map->empty == map->full ||
        map->obstacle == map->full)
        return (0);

    return (1);
}

// -------------------- READ MAP --------------------
int read_map(FILE *fp, t_map *map)
{
    char *line = NULL;
    size_t len = 0;
    int i = 0;

    map->grid = malloc(sizeof(char *) * map->rows);
    if (!map->grid)
        return (0);

    while (i < map->rows && getline(&line, &len, fp) != -1)
    {
        int l = strlen(line);
        if (line[l - 1] == '\n')
            line[l - 1] = '\0';

        if (i == 0)
            map->cols = strlen(line);
        else if ((int)strlen(line) != map->cols)
            return (free(line), 0);

        map->grid[i] = strdup(line);
        if (!map->grid[i])
            return (free(line), 0);

        for (int j = 0; j < map->cols; j++)
        {
            if (map->grid[i][j] != map->empty &&
                map->grid[i][j] != map->obstacle)
                return (free(line), 0);
        }
        i++;
    }
    free(line);
    return (i == map->rows);
}

// -------------------- CHECK SQUARE --------------------
int can_place_square(t_map *map, int row, int col, int size)
{
    for (int i = row; i < row + size; i++)
    {
        for (int j = col; j < col + size; j++)
        {
            if (i >= map->rows || j >= map->cols)
                return (0);
            if (map->grid[i][j] == map->obstacle)
                return (0);
        }
    }
    return (1);
}

// -------------------- SOLVE (BRUTE FORCE) --------------------
void solve_bsq(t_map *map)
{
    int max_size = 0;
    int best_row = 0;
    int best_col = 0;

    for (int i = 0; i < map->rows; i++)
    {
        for (int j = 0; j < map->cols; j++)
        {
            if (map->grid[i][j] == map->empty)
            {
                int size = 1;
                while (can_place_square(map, i, j, size))
                    size++;

                size--; // last valid size

                if (size > max_size)
                {
                    max_size = size;
                    best_row = i;
                    best_col = j;
                }
            }
        }
    }

    // Fill the best square
    for (int i = best_row; i < best_row + max_size; i++)
    {
        for (int j = best_col; j < best_col + max_size; j++)
            map->grid[i][j] = map->full;
    }
}

// -------------------- PRINT --------------------
void print_map(t_map *map)
{
    for (int i = 0; i < map->rows; i++)
        printf("%s\n", map->grid[i]);
}

// -------------------- PROCESS ONE FILE --------------------
int process_file(FILE *fp)
{
    t_map map = {0};
    char *line = NULL;
    size_t len = 0;

    if (getline(&line, &len, fp) == -1)
        return (free(line), map_error(&map));

    if (!parse_header(line, &map))
        return (free(line), map_error(&map));

    free(line);

    if (!read_map(fp, &map))
        return (map_error(&map));

    solve_bsq(&map);
    print_map(&map);
    free_map(&map);
    return (1);
}

// -------------------- MAIN --------------------
int main(int argc, char **argv)
{
    if (argc == 1)
    {
        process_file(stdin);
    }
    else
    {
        for (int i = 1; i < argc; i++)
        {
            FILE *fp = fopen(argv[i], "r");
            if (!fp)
            {
                fprintf(stderr, "map error\n");
                continue;
            }

            process_file(fp);
            fclose(fp);

            if (i < argc - 1)
                printf("\n");
        }
    }
    return (0);
}