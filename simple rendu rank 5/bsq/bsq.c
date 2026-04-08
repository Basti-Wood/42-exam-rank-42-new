#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int   rows;
    int   cols;
    char  empty;
    char  obstacle;
    char  full;
    char **grid;
} t_map;

void free_map(t_map *map)
{
    for (int i = 0; i < map->rows; i++)
        free(map->grid[i]);
    free(map->grid);
    map->grid = NULL;
}

int map_err(t_map *map)
{
    fprintf(stderr, "map error\n");
    if (map->grid)
        free_map(map);
    return 0;
}

int valid_header(char *line, t_map *map)
{
    if (sscanf(line, "%d %c %c %c", &map->rows, &map->empty, &map->obstacle, &map->full) != 4)
        return 0;
    if (map->rows <= 0)
        return 0;
    if (map->empty == map->obstacle || map->empty == map->full || map->obstacle == map->full)
        return 0;
    return 1;
}

int valid_line(char *line, t_map *map)
{
    for (int i = 0; line[i]; i++)
        if (line[i] != map->empty && line[i] != map->obstacle)
            return 0;
    return 1;
}

int parse_map(FILE *fp, t_map *map)
{
    char  *line = NULL;
    size_t len  = 0;

    if (getline(&line, &len, fp) == -1 || !valid_header(line, map))
        return free(line), map_err(map);
    free(line);

    map->grid = calloc(map->rows, sizeof(char *));
    if (!map->grid)
        return map_err(map);

    for (int i = 0; i < map->rows; i++)
    {
        line = NULL;
        len  = 0;
        if (getline(&line, &len, fp) == -1)
            return free(line), map_err(map);

        // strip newline
        int n = strlen(line);
        if (n > 0 && line[n - 1] == '\n')
            line[--n] = '\0';

        // set cols from first row, then check consistency
        if (i == 0)
            map->cols = n;
        if (n == 0 || n != map->cols || !valid_line(line, map))
            return free(line), map_err(map);

        map->grid[i] = line;
    }
    return 1;
}

void print_map(t_map *map)
{
    for (int i = 0; i < map->rows; i++)
        printf("%s\n", map->grid[i]);
}

int fits(t_map *map, int row, int col, int size)
{
    if (row + size > map->rows || col + size > map->cols)
        return 0;
    for (int r = row; r < row + size; r++)
        for (int c = col; c < col + size; c++)
            if (map->grid[r][c] == map->obstacle)
                return 0;
    return 1;
}

void solve(t_map *map)
{
    int br = 0, bc = 0, best = 0;

    for (int r = 0; r < map->rows; r++)
        for (int c = 0; c < map->cols; c++)
        {
            int s = 0;
            while (fits(map, r, c, s + 1))
                s++;
            if (s > best)
            {
                best = s;
                br = r;
                bc = c;
            }
        }
    for (int r = br; r < br + best; r++)
        for (int c = bc; c < bc + best; c++)
            map->grid[r][c] = map->full;
}

void process(FILE *fp)
{
    t_map map = {0};
    if (!parse_map(fp, &map))
        return;
    solve(&map);
    print_map(&map);
    free_map(&map);
}

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        process(stdin);
        return 0;
    }
    for (int i = 1; i < argc; i++)
    {
        FILE *fp = fopen(argv[i], "r");
        if (!fp)
            fprintf(stderr, "map error\n");
        else
        {
            process(fp);
            fclose(fp);
        }
        if (i < argc - 1)
            printf("\n");
    }
    return 0;
}

