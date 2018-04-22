#include "leptjson.h"
#include <assert.h> /* assert() */
#include <stdlib.h> /* NULL, strtod() */

#define EXPECT(c, ch)             \
    do                            \
    {                             \
        assert(*c->json == (ch)); \
        c->json++;                \
    } while (0)

#define is1Num(ch) (ch >= '1' && ch <= '9')
#define is0Num(ch) (ch >= '0' && ch <= '9')
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')
typedef struct
{
    const char *json;
} lept_context;

static void lept_parse_whitespace(lept_context *c)
{
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context *c, lept_value *v, lept_type type, const char *symbol)
{
    size_t i;
    EXPECT(c, *symbol);
    for (i = 0; symbol[i + 1]; i++)
    {
        if (c->json[i] != symbol[i + 1])
        {
            return LEPT_PARSE_INVALID_VALUE;
        }
    }
    c->json += i;
    v->type = type;
    return LEPT_PARSE_OK;
}

static int check_number(lept_context *c)
{
    /* 自动机 */
    const char *p = c->json;
    int hexFlag = 0;
    if (*p == '-')
        p++;
    if (*p == '0')
    {
        p++;
        hexFlag = 1;
    }
    else
    {
        if(!is1Num(*p)) {
            return 0;
        } else {
            for(p++; is0Num(*p); p++);
        }
    }
    if(hexFlag && (is0Num(*p) || *p == 'x')) {
        return 0;
    }
    if(*p == '.') {
        hexFlag = 1;
        p++;
        if(!is0Num(*p)) {
            return 0;
        } else {
            for(p++; is0Num(*p); p++);
        }
    }
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!is0Num(*p)) return 0;
        for (p++; is0Num(*p); p++);
    }

    return 1;
}

static int lept_parse_number(lept_context *c, lept_value *v)
{
    char *end;
    if (!check_number(c))
        return LEPT_PARSE_INVALID_VALUE;
    v->n = strtod(c->json, &end);
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_number2(lept_context* c, lept_value* v) {
    const char* p = c->json;
    char *end;
    if (*p == '-') p++;
    if (*p == '0') p++;
    else {
        if (!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    v->n = strtod(c->json, &end);
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}
static int lept_parse_value(lept_context *c, lept_value *v)
{
    switch (*c->json)
    {
    case 't':
        return lept_parse_literal(c, v, LEPT_TRUE, "true");
    case 'f':
        return lept_parse_literal(c, v, LEPT_FALSE, "false");
    case 'n':
        return lept_parse_literal(c, v, LEPT_NULL, "null");
    default:
        return lept_parse_number(c, v);
    case '\0':
        return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value *v, const char *json)
{
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK)
    {
        lept_parse_whitespace(&c);
        if (*c.json != '\0')
        {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value *v)
{
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value *v)
{
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
