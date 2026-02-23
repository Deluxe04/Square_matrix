#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "field.h"
#include "int_field.h"

static const FieldInfo* g_int_field_info = NULL;

static void IntReader(void* dest, FILE* src) {
    fscanf(src, "%d", (int*)dest);
}

static void IntPrinter(const void* data, FILE* dest) {
    fprintf(dest, "%d", *(const int*)data);
}

static void IntAdder(void* result, const void* a, const void* b) {
    *(int*)result = *(const int*)a + *(const int*)b;
}

static void IntSubtractor(void* result, const void* a, const void* b) {
    *(int*)result = *(const int*)a - *(const int*)b;
}

static void IntMultiplier(void* result, const void* a, const void* b) {
    *(int*)result = *(const int*)a * *(const int*)b;
}

static void IntDivider(void* result, const void* a, const void* b) {
    int divisor = *(const int*)b;
    if (divisor != 0) {
        *(int*)result = *(const int*)a / divisor;
    }
}

//Инициализация
static const FieldInfo* CreateIntFieldInfo(void) {
    FieldInfo* info = (FieldInfo*)malloc(sizeof(FieldInfo));
    if (!info) return NULL;
    
    info->size = sizeof(int);
    strcpy(info->name, "int");
    
    info->read = IntReader;
    info->print = IntPrinter;
    info->add = IntAdder;
    info->sub = IntSubtractor;  
    info->mul = IntMultiplier;
    info->div = IntDivider;      
    
    return info;
}

const FieldInfo* GetIntFieldInfo(void) {
    if (g_int_field_info == NULL) {
        g_int_field_info = CreateIntFieldInfo();
    }
    return g_int_field_info;
}
