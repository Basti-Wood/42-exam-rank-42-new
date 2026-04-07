#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

char **make_map(int height, int width)
{
    char **map = calloc(height, sizeof(char*));
    if (!map)
    {
        perror("calloc failed for map");
        return NULL;
    }

    for (int i = 0; i < height; i++)
    {
        map[i] = calloc(width, sizeof(char));
        if (!map[i])
        {
            perror("calloc failed for map row");
            for (int j = 0; j < i; j++)
                free(map[j]);
            free(map);
            return NULL;
        }
    }
    return map;
}

void	read_input(char **board, int width, int height)
{
	int		row;
	int		col;
	bool	pen;
	char	c;

	row = 0;
	col = 0;
	pen = false;
	while (read(0, &c, 1) == 1)
	{
		if (c == 'x')
			pen = !pen;
		else if (c == 'w' && row > 0)
			row--;
		else if (c == 's' && row < height - 1)
			row++;
		else if (c == 'a' && col > 0)
			col--;
		else if (c == 'd' && col < width - 1)
			col++;
		else
			continue;
		if (pen)
			board[row][col] = 1;
	}
}

void	free_board(char **board, int height)
{
	int	i;

	i = 0;
	while (i < height)
		free(board[i++]);
	free(board);
}

int count_neighbors(char **map, int wdth, int height, int row, int col)
{
    int count = 0;
    int r = row - 1;
    int c;
    while (r <= row + 1)
    {
        c = col - 1;
        while (c <= col + 1)
        {
            if ((r != row || c != col) && r >= 0 && r < height && c >= 0 && c < wdth)
                count += map[r][c];
            c++;
        }
        r++;
    }
    return count;
}

char **next_gen(char **map, int width, int height)
{
    char **next = make_map(height, width);
    int row = 0;
    int col = 0;
    int n;

    if (!next)
		return (NULL);
    while(row < height)
    {
        col = 0;
        while (col < width)
        {
            n = count_neighbors(map, width, height, row, col);
            if (map[row][col])
            {
                if(n==2 || n == 3)
                    next[row][col] = 1;
            }
            else
            {
                if(n == 3)
                    next[row][col] = 1;
            }
            ++col;
        }
        ++row;
    }
    return next;
}

void print_map(char **map, int height, int width)
{
    int row = 0;
    int col = 0;

    while(row < height)
    {
        col = 0;
        while(col < width)
        {
            if(map[row][col])
                putchar('0');
            else
                putchar(' ');
            col++;
        }
        putchar('\n');
        row++;
    }
}

int main(int argv, char **argc)
{
    if(argv != 4)
    {
        printf("ERROR %s Width height iterations\n", argc[0]);
        return 1;
    }
    int height = atoi(argc[2]);
    int width = atoi(argc[1]);
    int iter = atoi(argc[3]);
    char **map = make_map(height, width);
    char **next;
    read_input(map, width, height);
    while(iter > 0)
    {
		next = next_gen(map, width, height);
		if (!next)
			break ;
		free_board(map, height);
		map = next;
		iter--;
    }
    print_map(map, height, width);
    free_board(map, height);

    return 0;
}