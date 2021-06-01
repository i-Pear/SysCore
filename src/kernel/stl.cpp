#include "../lib/stl/stl.h"

void memcpy(void *to, const void *from, size_t size) {
    char *tar = (char*)to;
    const char *ori = (const char*)from;
    for (size_t i = 0; i < size; i++) {
        *(tar + i) = *(ori + i);
    }
}

void memset(void *p, char content, size_t size) {
    for (int i = 0; i < size; i++) {
        *((char *) p + i) = content;
    }
}

int strlen(const char *s) {
    int n;
    for (n = 0; s[n]; n++);
    return n;
}

void strcpy(char* to,const char* from){
    int len = strlen(from);
    memcpy(to, from, len * sizeof(char));
    to[len] = '\0';
}

int strcmp(const char *cs, const char *ct)
{
    unsigned char c1, c2;
    while (true) {
        c1 = *cs++;
        c2 = *ct++;
        if (c1 != c2)
            return c1 < c2 ? -1 : 1;
        if (!c1)
            break;
    }
    return 0;
}
