#ifndef OS_RISC_V_PATHUTIL_H
#define OS_RISC_V_PATHUTIL_H

#include "list.h"
#include "string.h"

class PathUtil {
private:
    /**
 * strpbrk - Find the first occurrence of a set of characters
 * @cs: The string to be searched
 * @ct: The characters to search for
 */
    static char *strpbrk(const char *cs, const char *ct) {
        const char *sc1, *sc2;

        for (sc1 = cs; *sc1 != '\0'; ++sc1) {
            for (sc2 = ct; *sc2 != '\0'; ++sc2) {
                if (*sc1 == *sc2)
                    return (char *) sc1;
            }
        }
        return NULL;
    }

/**
 * strsep - Split a string into tokens
 * @s: The string to be searched
 * @ct: The characters to search for
 *
 * strsep() updates @s to point after the token, ready for the next call.
 *
 * It returns empty tokens, too, behaving exactly like the libc function
 * of that name. In fact, it was stolen from glibc2 and de-fancy-fied.
 * Same semantics, slimmer shape. ;)
 */
    static char *strsep(char **s, const char *ct) {
        char *sbegin = *s;
        char *end;

        if (sbegin == NULL)
            return NULL;

        end = strpbrk(sbegin, ct);
        if (end)
            *end++ = '\0';
        *s = end;
        return sbegin;
    }

public:
    static List<String> split(const String &path) {
        char *unit;
        List<String> res;
        char *c_str = path.c_str();
        while ((unit = strsep(&c_str, "/")) != NULL) {
            if (*unit != '\0') {
                res.push_back(unit);
            }
        }
        return res;
    }

    static String joinAbsolutePath(const List<String> &pathList) {
        String res;
        auto p = pathList.start;
        while (p) {
            res += "/";
            res += p->data;
            p = p->next;
        }
        return res;
    }
};

class TestPathUtil {
public:
    TestPathUtil() {
        auto list = PathUtil::split("/dev/sdb");
        assert(list.start->data == "dev" && list.start->next->data == "sdb");

        list = PathUtil::split("dev/sdb");
        assert(list.start->data == "dev" && list.start->next->data == "sdb");

        assert(PathUtil::joinAbsolutePath(list) == "/dev/sdb");
    }
};

#endif //OS_RISC_V_PATHUTIL_H
