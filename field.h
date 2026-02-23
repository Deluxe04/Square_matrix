#ifndef FIELD_H
#define FIELD_H

#include <stdio.h>

typedef void (*FieldReaderFunc)(void* dest, FILE* src);
typedef void (*FieldPrinterFunc)(const void* data, FILE* dest);
typedef void (*FieldBinaryOpFunc)(void* result, const void* a, const void* b);

typedef struct FieldInfo FieldInfo;

struct FieldInfo {
    size_t size;
    char name[16];
    
    FieldReaderFunc read;      
    FieldPrinterFunc print; 
    

    FieldBinaryOpFunc add;
    FieldBinaryOpFunc sub;  
    FieldBinaryOpFunc mul;
    FieldBinaryOpFunc div;      
};

int FieldInfo_Equals(const FieldInfo* a, const FieldInfo* b);

#endif