#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	int		rows;
	int		cols;
	char	emp;
	char	obs;
	char	ful;
	char	**grid;
}	t_map;

static void	free_map(t_map *m)
{
	if (!m)
		return ;
	if (m->grid)
	{
		for (int i = 0; i < m->rows; i++)
			free(m->grid[i]);
		free(m->grid);
	}
	free(m);
}

static t_map	*parse_map(FILE *f)
{
	t_map	*m;
	char	*line = NULL;
	size_t	cap = 0;
	int		len;
	char	eol;

	m = calloc(1, sizeof(t_map));
	if (fscanf(f, "%d %c %c %c%c", &m->rows, &m->emp, &m->obs, &m->ful, &eol) != 5
		|| eol != '\n' || m->rows <= 0
		|| m->emp == m->obs || m->emp == m->ful || m->obs == m->ful)
		return (free(m), NULL);
	m->grid = calloc(m->rows, sizeof(char *));
	for (int i = 0; i < m->rows; i++)
	{
		len = (int)getline(&line, &cap, f);
		if (len <= 0)
			{ free(line); free_map(m); return (NULL); }
		if (line[len - 1] == '\n')
			line[--len] = '\0';
		if (len == 0)
			{ free(line); free_map(m); return (NULL); }
		if (i == 0)
			m->cols = len;
		else if (len != m->cols)
			{ free(line); free_map(m); return (NULL); }
		for (int j = 0; j < m->cols; j++)
			if (line[j] != m->emp && line[j] != m->obs)
				{ free(line); free_map(m); return (NULL); }
		m->grid[i] = strdup(line);
	}
	free(line);
	return (m);
}
static int	is_square(t_map *m, int r, int c, int s)
{
	for (int i = r; i < r + s; i++)
		for (int j = c; j < c + s; j++)
			if (m->grid[i][j] == m->obs)
				return (0);
	return (1);
}

static void	solve(t_map *m)
{
	int	best = 0, br = 0, bc = 0;
	int	max_s = m->rows < m->cols ? m->rows : m->cols;

	for (int s = max_s; s >= 1; s--)
	{
		for (int i = 0; i <= m->rows - s; i++)
		{
			for (int j = 0; j <= m->cols - s; j++)
			{
				if (is_square(m, i, j, s))
				{
					best = s; br = i; bc = j;
					goto found;
				}
			}
		}
	}
	found:
	for (int i = br; i < br + best; i++)
		for (int j = bc; j < bc + best; j++)
			m->grid[i][j] = m->ful;
	for (int i = 0; i < m->rows; i++)
	{
		fputs(m->grid[i], stdout);
		fputc('\n', stdout);
	}
}

static void	run(FILE *f)
{
	t_map	*m;

	m = parse_map(f);
	if (!m)
		fprintf(stderr, "map error\n");
	else
	{
		solve(m);
		free_map(m);
	}
}

int	main(int argc, char **argv)
{
	if (argc == 1)
		run(stdin);
	else
	{
		for (int i = 1; i < argc; i++)
		{
			FILE *f = fopen(argv[i], "r");
			if (!f)
				fprintf(stderr, "map error\n");
			else
			{
				run(f);
				fclose(f);
			}
			if (i < argc - 1)
				fputc('\n', stdout);
		}
	}
	return (0);
}
