#ifndef MATRIX_H
#define MATRIX_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include "field.h"  

typedef enum {
    MATRIX_OK = 0,
    MATRIX_ERROR_NULL_POINTER = -1,
    MATRIX_ERROR_MEMORY = -2,
    MATRIX_ERROR_INVALID_SIZE = -3,
    MATRIX_ERROR_TYPE_MISMATCH = -4,
    MATRIX_ERROR_DIMENSION_MISMATCH = -5,
    MATRIX_ERROR_INVALID_INDEX = -6,
    MATRIX_ERROR_SINGULAR_MATRIX = -7
} MatrixError;

typedef struct {
    void* data;
    size_t rows;
    size_t cols;
    const FieldInfo* type;  
} Matrix;

Matrix* Matrix_Create(size_t rows, size_t cols, const FieldInfo* type);
void Matrix_Destroy(Matrix* m);

MatrixError Matrix_Get(const Matrix* m, size_t row, size_t col, void* out);
MatrixError Matrix_Set(Matrix* m, size_t row, size_t col, const void* value);

Matrix* Matrix_Add(const Matrix* a, const Matrix* b, MatrixError* error);
Matrix* Matrix_Multiply(const Matrix* a, const Matrix* b, MatrixError* error);
Matrix* Matrix_ScalarMultiply(const Matrix* m, const void* scalar, MatrixError* error);
Matrix* Matrix_AddLinearCombination(const Matrix* m, size_t row_idx, 
                                    const void* alphas, MatrixError* error);

Matrix* Matrix_Clone(const Matrix* m, MatrixError* error);
MatrixError Matrix_Fill(Matrix* m, const void* value);
MatrixError Matrix_Identity(Matrix* m);

MatrixError Matrix_Print(const Matrix* m, const char* name, FILE* output);
Matrix* Matrix_Read(FILE* input, const FieldInfo* type, MatrixError* error);

MatrixError Matrix_GaussSolve(const Matrix* a, const Matrix* b, Matrix* x);

const char* Matrix_ErrorString(MatrixError error);

#endif
