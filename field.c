#include "field.h"
#include <string.h>

int FieldInfo_Equals(const FieldInfo* a, const FieldInfo* b) {
    if (a == b) return 1;
    if (!a || !b) return 0;
    return (a->size == b->size) && (strcmp(a->name, b->name) == 0);
}
