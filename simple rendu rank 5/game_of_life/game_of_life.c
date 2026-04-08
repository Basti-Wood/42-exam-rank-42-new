#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

bool **make_map(int height, int width)
{
    bool** map = calloc(height, sizeof(bool**));
    if(!map)
    {
        free(map);
        return NULL;
    }
    for(int i = 0; i < height; i++)
    {
        map[i] = calloc(width, sizeof(bool*));
        if(!map[i])
        {
            while(i > 0)
            {
                free(map[i]);
                i--;
            }
            free(map);
            return NULL;
        }
    }
    return map;
}

void free_map(bool **map, int height)
{
    for(int i = 0; i < height; i++)
    {
        free(map[i]);
    }
    free(map);
}

void read_input(bool **map, int height, int width)
{
    char c = 0;
    bool pen = false;
    int row = 0;
    int col = 0;
    while(read(0, &c, 1) == 1)
    {
        if(c == 'x')
            pen = !pen;
        else if(c == 'w' && row > 0)
           row--;
        else if(c == 'a' && col > 0)
            col--;
        else if(c == 's' && row < height)
            row++;
        else if(c == 'd' && col < width)
            col++;
        else
            continue;
        if(pen)
            map[row][col] = 1;
        
    }
}

int count_neighbors(bool **map, int height, int width, int row, int col)
{
    int count = 0;
    for(int r = row-1; r <= row+1; r++)
    {
        for(int c = col-1; c <= col+1; c++)
        {
            if((r != row || c != col) && r >= 0 && c >= 0 && r < height && c < width)
                count += map[r][c];
        }
    }
    return count;
}

bool **next_gen(bool **map, int height, int width)
{
    int count = 0;
    bool **next = make_map(height, width);
    for(int row = 0; row < height; row++)
    {
        for(int col = 0; col < width; col++)
        {
            count = count_neighbors(map, height, width, row, col);
            if((map[row][col] && count == 2) || count == 3)
                next[row][col] = 1;
        }
    }
    return next;
}

void print_map(bool** map, int height, int width)
{
    for(int row = 0; row < height; row++)
    {
        for(int col = 0; col < width; col++)
        {
            if(map[row][col])
                putchar('0');
            else
                putchar(' ');
        }
        putchar('\n');
    }
}

int main(int i, char **c)
{
    if(i != 4)
        return 0;
    int width = atoi(c[1]);
    int height = atoi(c[2]);
    int iter = atoi(c[3]);
    bool** map = make_map(height, width);
    bool** next;
    read_input(map, height, width);
    while(iter > 0)
    {
        next = next_gen(map, height, width);
        free_map(map, height);
        map = next;
        iter--;
    }
    print_map(map, height, width);
    free_map(map, height);
    return 0;
}