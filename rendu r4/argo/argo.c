#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

typedef struct  json {
        enum {
                MAP,
                INTEGER,
                STRING
        } type;
        union {
                struct {
                        struct pair     *data;
                        size_t          size;
                } map;
                int     integer;
                char    *string;
        };
}       json;

typedef struct  pair {
        char    *key;
        json    value;
}       pair;

int     peek(FILE *stream)
{
        int     c = getc(stream);
        ungetc(c, stream);
        return c;
}

void    unexpected(FILE *stream)
{
        if (peek(stream) != EOF)
                printf("unexpected token '%c'\n", peek(stream));
        else
                printf("unexpected end of input\n");
}

int     accept(FILE *stream, char c)
{
        if (peek(stream) == c)
        {
                (void)getc(stream);
                return 1;
        }
        return 0;
}

int     expect(FILE *stream, char c)
{
        if (accept(stream, c))
                return 1;
        unexpected(stream);
        return 0;
}

void    free_json(json j)
{
        switch (j.type)
        {
                case MAP:
                        for (size_t i = 0; i < j.map.size; i++)
                        {
                                free(j.map.data[i].key);
                                free_json(j.map.data[i].value);
                        }
                        free(j.map.data);
                        break ;
                case STRING:
                        free(j.string);
                        break ;
                default:
                        break ;
        }
}

void    ft_serialize(json j)
{
        switch (j.type)
        {
                case INTEGER:
                        printf("%d", j.integer);
                        break ;
                case STRING:
                        putchar('"');
                        for (int i = 0; j.string[i]; i++)
                        {
                                if (j.string[i] == '\\' || j.string[i] == '"')
                                        putchar('\\');
                                putchar(j.string[i]);
                        }
                        putchar('"');
                        break ;
                case MAP:
                        putchar('{');
                        for (size_t i = 0; i < j.map.size; i++)
                        {
                                if (i != 0)
                                        putchar(',');
                                ft_serialize((json){.type = STRING, .string = j.map.data[i].key});
                                putchar(':');
                                ft_serialize(j.map.data[i].value);
                        }
                        putchar('}');
                        break ;
        }
}
int		parse_string(json *dst, FILE *stream);
int		parse_value(json *dst, FILE *stream);
int		parse_map(json *dst, FILE *stream);
int		argo(json *dst, FILE *stream);

int		parse_string(json *dst, FILE *stream)
{
	if (!expect(stream, '"'))
		return -1;
	char	*str = NULL;
	size_t	size = 0;
	size_t	capacity = 0;
	char 	  c;
	while ((c = getc(stream)) != '"')
	{
		if (c == EOF)
		{
			unexpected(stream);
			free(str);
			return -1;
		}
		if (c == '\\')
		{
			c = getc(stream);
			if (c == EOF)
			{
				unexpected(stream);
				free(str);
				return -1;
			}
		}
		if (size + 1 >= capacity)
		{
			if (capacity == 0)
				capacity = 16;
			else
				capacity *= 2;
			str = realloc(str, capacity);
		}
		str[size++] = c;
	}
	str[size] = '\0';
	dst->type = STRING;
	dst->string = str;
	return 1;
}

int parse_value(json *dst, FILE *stream)
{
	int num;
	if (fscanf(stream, "%d", &num) != 1)
	{
		unexpected(stream);
		return -1;
	}
	dst->type = INTEGER;
	dst->integer = num;
	return 1;
}

int parse_map(json *dst, FILE *stream)
{
	if (!expect(stream, '{'))
		return -1;
	dst->type = MAP;
	dst->map.data = NULL;
	dst->map.size = 0;
	if (accept(stream, '}'))
		return 1;
	while(1)
	{
		json key_json;
		if (parse_string(&key_json, stream) != 1)
			return -1;
		if (!expect(stream, ':'))
		{
			free(key_json.string);
			return -1;
		}
		dst->map.data = realloc(dst->map.data, sizeof(pair) * (dst->map.size + 1));
		dst->map.data[dst->map.size].key = key_json.string;
		if (argo(&dst->map.data[dst->map.size].value, stream) != 1)
			return -1;
		dst->map.size++;
		if (accept(stream, '}'))
			return 1;
		if (!expect(stream, ','))
			return -1;
	}
}

int     argo(json *dst, FILE *stream)
{
	char c = peek(stream);
	if (c == '"')
		return parse_string(dst, stream);
	else if (c == '{')
		return parse_map(dst, stream);
	else if (isdigit(c) || c == '-')
		return parse_value(dst, stream);
	else
	{
		unexpected(stream);
		return -1;
	}
}
/*
int     main(int argc, char **argv)
{
        if (argc != 2)
                return 1;
        char *filename = argv[1];
        FILE *stream = fopen(filename, "r");
        json    file;
        if (argo (&file, stream) != 1)
        {
                free_json(file);
                return 1;
        }
        ft_serialize(file);
        printf("\n");
}*/