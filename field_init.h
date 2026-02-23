#ifndef FIELD_INIT_H
#define FIELD_INIT_H

void InitIntField(void);
void InitFloatField(void);

static inline void InitAllFields(void) {
    InitIntField();
    InitFloatField();
}

#endif