#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "field.h"
#include "float_field.h"

static const FieldInfo* g_float_field_info = NULL;

static void FloatReader(void* dest, FILE* src) {
    fscanf(src, "%f", (float*)dest);
}

static void FloatPrinter(const void* data, FILE* dest) {
    fprintf(dest, "%.2f", *(const float*)data);
}

static void FloatAdder(void* result, const void* a, const void* b) {
    *(float*)result = *(const float*)a + *(const float*)b;
}

static void FloatSubtractor(void* result, const void* a, const void* b) {
    *(float*)result = *(const float*)a - *(const float*)b;
}

static void FloatMultiplier(void* result, const void* a, const void* b) {
    *(float*)result = *(const float*)a * *(const float*)b;
}

static void FloatDivider(void* result, const void* a, const void* b) {
    float divisor = *(const float*)b;
    if (divisor != 0.0f) {
        *(float*)result = *(const float*)a / divisor;
    }
}

//Инициализация
static const FieldInfo* CreateFloatFieldInfo(void) {
    FieldInfo* info = (FieldInfo*)malloc(sizeof(FieldInfo));
    if (!info) return NULL;
    
    info->size = sizeof(float);
    strcpy(info->name, "float");
    
    info->read = FloatReader;
    info->print = FloatPrinter;
    info->add = FloatAdder;
    info->sub = FloatSubtractor;  
    info->mul = FloatMultiplier;
    info->div = FloatDivider;      
    
    return info;
}

const FieldInfo* GetFloatFieldInfo(void) {
    if (g_float_field_info == NULL) {
        g_float_field_info = CreateFloatFieldInfo();
    }
    return g_float_field_info;
}

