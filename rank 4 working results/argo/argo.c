#include "argo.h"

/*
 * argo.c - sehr einfacher JSON-Parser/Serializer (Teilmenge)
 *
 * Unterstützte Werte: INTEGER, STRING, MAP (Objekt mit string-keys)
 * - Keine Arrays, keine true/false/null, keine Fließkommazahlen
 * - Keine Whitespace außer dort, wo explizit erwartet wird (dieser Parser
 *   behandelt Whitespace außerhalb von strings als Fehler)
 *
 * Dateiaufbau:
 * - peek/accept/expect: kleine Hilfsfunktionen zum Einlesen/Prüfen
 * - parse_*: rekursive Descent-Funktionen für string, integer, map
 * - free_json / serialize: Aufräumen und Serialisierung zurück zu JSON
 * - argo: API-Funktion, die einen json-Wert aus einem FILE* parst
 */

/* Lies ein Zeichen und lege es wieder zurück in den Stream (peek) */
int peek(FILE *stream)
{
    int c = getc(stream);
    ungetc(c, stream);
    return c;
}

/* Meldet ein unerwartetes Token (Fehlerausgabe) */
void unexpected(FILE *stream)
{
    if (peek(stream) != EOF)
        printf("unexpected token '%c'\n", peek(stream));
    else
        printf("unexpected end of input\n");
}

/* Wenn das nächste Zeichen `c` ist, konsumiere es und gib 1 zurück */
int accept(FILE *stream, char c)
{
    if (peek(stream) == c)
    {
        (void)getc(stream);
        return 1;
    }
    return 0;
}

/* Erwarte ein bestimmtes Zeichen; wenn nicht vorhanden -> Fehlerausgabe */
int expect(FILE *stream, char c)
{
    if (accept(stream, c))
        return 1;
    unexpected(stream);
    return 0;
}

/* Rekursive Freigabe eines json-Objekts
 * - Gibt bei MAP alle Keys und die zugehörigen Werte rekursiv frei
 * - Bei STRING wird der zugewiesene String freigegeben
 */
void free_json(json j)
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
            break;
        case STRING:
            free(j.string);
            break;
        default:
            break;
    }
}

/* Serialisiere ein json-Objekt wieder als JSON-Text (vereinfachte Ausgabe) */
void serialize(json j)
{
    switch (j.type)
    {
        case INTEGER:
            printf("%d", j.integer);
            break;
        case STRING:
            putchar('"');
            /* Escapen von \ und " im String */
            for (int i = 0; j.string[i]; i++)
            {
                if (j.string[i] == '\\' || j.string[i] == '"')
                    putchar('\\');
                putchar(j.string[i]);
            }
            putchar('"');
            break;
        case MAP:
            putchar('{');
            for (size_t i = 0; i < j.map.size; i++)
            {
                if (i != 0)
                    putchar(',');
                /* Keys sind intern als C-Strings gespeichert; wir
                   serialisieren sie als JSON-Strings */
                serialize((json){.type = STRING, .string = j.map.data[i].key});
                putchar(':');
                serialize(j.map.data[i].value);
            }
            putchar('}');
            break;
    }
}

/* Vorwärtsdeklaration für rekursive Abhängigkeiten */
static int parse_value(json *dst, FILE *stream);

/* Parse einen JSON-String (unterstützt einfache Escapes \" und \\\\) */
static int parse_string(json *dst, FILE *stream)
{
    if (!accept(stream, '"'))
        return 0;
    size_t cap = 32;
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf)
        return 0;
    for (;;)
    {
        int c = getc(stream);
        if (c == EOF)
        {
            free(buf);
            unexpected(stream);
            return 0;
        }
        if (c == '"')
            break; /* Ende des Strings */
        if (c == '\\')
        {
            /* Escape-Sequenz: wir unterstützen mindestens \" und \\\\ */
            int n = getc(stream);
            if (n == EOF)
            {
                free(buf);
                unexpected(stream);
                return 0;
            }
            /* Bei anderen Escapes werden die Zeichen aktuell wörtlich übernommen */
            c = n;
        }
        if (len + 1 >= cap)
        {
            cap *= 2;
            char *tmp = realloc(buf, cap);
            if (!tmp)
            {
                free(buf);
                return 0;
            }
            buf = tmp;
        }
        buf[len++] = (char)c;
    }
    buf[len] = '\0';
    dst->type = STRING;
    dst->string = buf;
    return 1;
}

/* Parse ganze (optionale Vorzeichen) Integer Zahlen */
static int parse_integer(json *dst, FILE *stream)
{
    int c = peek(stream);
    int sign = 1;
    if (c == '-')
    {
        (void)getc(stream);
        sign = -1;
        c = peek(stream);
    }
    if (!isdigit(c))
    {
        unexpected(stream);
        return 0;
    }
    long val = 0;
    for (;;)
    {
        c = peek(stream);
        if (!isdigit(c))
            break;
        c = getc(stream);
        val = val * 10 + (c - '0');
    }
    dst->type = INTEGER;
    dst->integer = (int)(val * sign);
    return 1;
}

/* Parse eines JSON-Objekts (Map): { "key": value, ... } */
static int parse_map(json *dst, FILE *stream)
{
    if (!accept(stream, '{'))
        return 0;
    /* initialize empty map */
    dst->type = MAP;
    dst->map.data = NULL;
    dst->map.size = 0;

    /* leeres Objekt {} */
    if (accept(stream, '}'))
        return 1;

    for (;;)
    {
        /* keys must be strings */
        json key = {0};
        if (!parse_string(&key, stream))
        {
            /* parse_string gibt bereits eine Fehlermeldung aus; aufräumen */
            for (size_t i = 0; i < dst->map.size; i++)
            {
                free(dst->map.data[i].key);
                free_json(dst->map.data[i].value);
            }
            free(dst->map.data);
            return 0;
        }
        if (!expect(stream, ':'))
        {
            free(key.string);
            for (size_t i = 0; i < dst->map.size; i++)
            {
                free(dst->map.data[i].key);
                free_json(dst->map.data[i].value);
            }
            free(dst->map.data);
            return 0;
        }
        json value = {0};
        if (!parse_value(&value, stream))
        {
            free(key.string);
            for (size_t i = 0; i < dst->map.size; i++)
            {
                free(dst->map.data[i].key);
                free_json(dst->map.data[i].value);
            }
            free(dst->map.data);
            return 0;
        }
        /* append pair */
        pair *tmp = realloc(dst->map.data, sizeof(pair) * (dst->map.size + 1));
        if (!tmp)
        {
            free(key.string);
            free_json(value);
            for (size_t i = 0; i < dst->map.size; i++)
            {
                free(dst->map.data[i].key);
                free_json(dst->map.data[i].value);
            }
            free(dst->map.data);
            return 0;
        }
        dst->map.data = tmp;
        dst->map.data[dst->map.size].key = key.string;
        dst->map.data[dst->map.size].value = value;
        dst->map.size++;

        /* Ende des Objekts oder weiteres Paar */
        if (accept(stream, '}'))
            break;
        if (accept(stream, ','))
            continue;
        /* unerwartetes Zeichen */
        unexpected(stream);
        for (size_t i = 0; i < dst->map.size; i++)
        {
            free(dst->map.data[i].key);
            free_json(dst->map.data[i].value);
        }
        free(dst->map.data);
        return 0;
    }
    return 1;
}

/* Parse eines beliebigen Wertes (String | Map | Integer) */
static int parse_value(json *dst, FILE *stream)
{
    int c = peek(stream);
    if (c == EOF)
    {
        unexpected(stream);
        return 0;
    }
    if (c == '"')
        return parse_string(dst, stream);
    if (c == '{')
        return parse_map(dst, stream);
    if (c == '-' || isdigit(c))
        return parse_integer(dst, stream);
    /* Any whitespace outside strings is invalid for this parser */
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
    {
        unexpected(stream);
        return 0;
    }
    unexpected(stream);
    return 0;
}

/* Öffentliche API: parst einen JSON-Wert aus stream nach dst
 * Rückgabe: 1 bei Erfolg, -1 bei Fehlern
 */
int argo(json *dst, FILE *stream)
{
    if (!dst || !stream)
        return -1;
    /* initialize dst to safe default so caller can free on failure */
    *dst = (json){.type = INTEGER, .integer = 0};

    if (!parse_value(dst, stream))
    {
        /* on failure, ensure dst is a non-map so free_json is safe */
        *dst = (json){.type = INTEGER, .integer = 0};
        return -1;
    }
    if (peek(stream) != EOF)
    {
        unexpected(stream);
        free_json(*dst);
        *dst = (json){.type = INTEGER, .integer = 0};
        return -1;
    }
    return 1;
}

/* Einfache main-Funktion zum Parsen einer Datei und Ausgabe */
int main(int argc, char **argv)
{
    if (argc != 2)
        return 1;
    char *filename = argv[1];
    FILE *stream = fopen(filename, "r");
    json file;
    if (argo(&file, stream) != 1)
    {
        free_json(file);
        return 1;
    }
    serialize(file);
    printf("\n");
}