#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char	**create_board(int width, int height)
{
	char	**board;
	int		i;

	board = calloc(height, sizeof(char *));
	if (!board)
		return (NULL);
	i = 0;
	while (i < height)
	{
		board[i] = calloc(width, sizeof(char));
		if (!board[i])
		{
			while (i-- > 0)
				free(board[i]);
			free(board);
			return (NULL);
		}
		i++;
	}
	return (board);
}

void	free_board(char **board, int height)
{
	int	i;

	i = 0;
	while (i < height)
		free(board[i++]);
	free(board);
}

int	count_neighbors(char **board, int width, int height, int row, int col)
{
	int	count;
	int	r;
	int	c;

	count = 0;
	r = row - 1;
	while (r <= row + 1)
	{
		c = col - 1;
		while (c <= col + 1)
		{
			if ((r != row || c != col)
				&& r >= 0 && r < height && c >= 0 && c < width)
				count += board[r][c];
			c++;
		}
		r++;
	}
	return (count);
}

char	**next_generation(char **board, int width, int height)
{
	char	**next;
	int		row;
	int		col;
	int		n;

	next = create_board(width, height);
	if (!next)
		return (NULL);
	row = 0;
	while (row < height)
	{
		col = 0;
		while (col < width)
		{
			n = count_neighbors(board, width, height, row, col);
			if (board[row][col] ? (n == 2 || n == 3) : (n == 3))
				next[row][col] = 1;
			col++;
		}
		row++;
	}
	return (next);
}

void	read_input(char **board, int width, int height)
{
	int		row;
	int		col;
	int		pen;
	char	c;

	row = 0;
	col = 0;
	pen = 0;
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

int	main(int argc, char **argv)
{
	int		width;
	int		height;
	int		iterations;
	int		i;
	char	**board;
	char	**next;

	if (argc != 4)
		return (1);
	width = atoi(argv[1]);
	height = atoi(argv[2]);
	iterations = atoi(argv[3]);
	board = create_board(width, height);
	if (!board)
		return (1);
	read_input(board, width, height);
	i = 0;
	while (i++ < iterations)
	{
		next = next_generation(board, width, height);
		if (!next)
			break ;
		free_board(board, height);
		board = next;
	}
	i = 0;
	while (i < height)
	{
		int	col = 0;
		while (col < width)
			putchar(board[i][col++] ? '0' : ' ');
		putchar('\n');
		i++;
	}
	free_board(board, height);
	return (0);
}
