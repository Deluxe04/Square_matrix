#include <stdlib.h>
#include <string.h>
#include "matrix.h"
#include "field.h"         
#include "int_field.h"      
#include "float_field.h" 

static size_t matrix_index(const Matrix* m, size_t row, size_t col) {
    return row * m->cols + col;
}

static void* matrix_element_ptr(const Matrix* m, size_t row, size_t col) {
    size_t idx = matrix_index(m, row, col);
    return (char*)m->data + idx * m->type->size;
}

static bool types_compatible(const Matrix* a, const Matrix* b) {
    return FieldInfo_Equals(a->type, b->type);
}

Matrix* Matrix_Create(size_t rows, size_t cols, const FieldInfo* type) {
    if (!type || rows == 0 || cols == 0) return NULL;
    
    Matrix* m = (Matrix*)malloc(sizeof(Matrix));
    if (!m) return NULL;
    
    m->rows = rows;
    m->cols = cols;
    m->type = type;
    
    size_t total = rows * cols * type->size;
    m->data = malloc(total);
    if (!m->data) {
        free(m);
        return NULL;
    }
    
    memset(m->data, 0, total);
    return m;
}

void Matrix_Destroy(Matrix* m) {
    if (m) {
        free(m->data);
        free(m);
    }
}
//Ошибки
MatrixError Matrix_Get(const Matrix* m, size_t row, size_t col, void* out) {
    if (!m || !out) return MATRIX_ERROR_NULL_POINTER;
    if (!m->data || !m->type) return MATRIX_ERROR_TYPE_MISMATCH;
    if (row >= m->rows || col >= m->cols) return MATRIX_ERROR_INVALID_INDEX;
    
    void* ptr = matrix_element_ptr(m, row, col);
    memcpy(out, ptr, m->type->size);
    return MATRIX_OK;
}

MatrixError Matrix_Set(Matrix* m, size_t row, size_t col, const void* value) {
    if (!m || !value) return MATRIX_ERROR_NULL_POINTER;
    if (!m->data || !m->type) return MATRIX_ERROR_TYPE_MISMATCH;
    if (row >= m->rows || col >= m->cols) return MATRIX_ERROR_INVALID_INDEX;
    
    void* ptr = matrix_element_ptr(m, row, col);
    memcpy(ptr, value, m->type->size);
    return MATRIX_OK;
}

Matrix* Matrix_Add(const Matrix* a, const Matrix* b, MatrixError* error) {
    if (error) *error = MATRIX_OK;
    
    if (!a || !b) {
        if (error) *error = MATRIX_ERROR_NULL_POINTER;
        return NULL;
    }
    
    if (!types_compatible(a, b)) {
        if (error) *error = MATRIX_ERROR_TYPE_MISMATCH;
        return NULL;
    }
    
    if (a->rows != b->rows || a->cols != b->cols) {
        if (error) *error = MATRIX_ERROR_DIMENSION_MISMATCH;
        return NULL;
    }
    
    Matrix* result = Matrix_Create(a->rows, a->cols, a->type);
    if (!result) {
        if (error) *error = MATRIX_ERROR_MEMORY;
        return NULL;
    }
    
    size_t total = a->rows * a->cols;
    for (size_t i = 0; i < total; i++) {
        void* a_ptr = (char*)a->data + i * a->type->size;
        void* b_ptr = (char*)b->data + i * b->type->size;
        void* r_ptr = (char*)result->data + i * result->type->size;
        a->type->add(r_ptr, a_ptr, b_ptr);
    }
    
    return result;
}

Matrix* Matrix_Multiply(const Matrix* a, const Matrix* b, MatrixError* error) {
    if (error) *error = MATRIX_OK;
    
    if (!a || !b) {
        if (error) *error = MATRIX_ERROR_NULL_POINTER;
        return NULL;
    }
    
    if (!types_compatible(a, b)) {
        if (error) *error = MATRIX_ERROR_TYPE_MISMATCH;
        return NULL;
    }
    
    if (a->cols != b->rows) {
        if (error) *error = MATRIX_ERROR_DIMENSION_MISMATCH;
        return NULL;
    }
    
    Matrix* result = Matrix_Create(a->rows, b->cols, a->type);
    if (!result) {
        if (error) *error = MATRIX_ERROR_MEMORY;
        return NULL;
    }
    
    char temp[16];
    
    for (size_t i = 0; i < a->rows; i++) {
        for (size_t j = 0; j < b->cols; j++) {
            void* r_ptr = matrix_element_ptr(result, i, j);
            memset(r_ptr, 0, a->type->size);
            
            for (size_t k = 0; k < a->cols; k++) {
                void* a_ptr = matrix_element_ptr(a, i, k);
                void* b_ptr = matrix_element_ptr(b, k, j);
                
                a->type->mul(temp, a_ptr, b_ptr);
                a->type->add(r_ptr, r_ptr, temp);
            }
        }
    }
    
    return result;
}


Matrix* Matrix_ScalarMultiply(const Matrix* m, const void* scalar, MatrixError* error) {
    if (error) *error = MATRIX_OK;
    
    if (!m || !scalar) {
        if (error) *error = MATRIX_ERROR_NULL_POINTER;
        return NULL;
    }
    
    Matrix* result = Matrix_Clone(m, error);
    if (!result) return NULL;
    
    size_t total = m->rows * m->cols;
    for (size_t i = 0; i < total; i++) {
        void* elem = (char*)result->data + i * result->type->size;
        result->type->mul(elem, elem, scalar);
    }
    
    return result;
}

Matrix* Matrix_AddLinearCombination(const Matrix* m, size_t row_idx, 
                                    const void* alphas, MatrixError* error) {
    if (error) *error = MATRIX_OK;
    
    if (!m || !alphas) {
        if (error) *error = MATRIX_ERROR_NULL_POINTER;
        return NULL;
    }
    
    if (row_idx >= m->rows) {
        if (error) *error = MATRIX_ERROR_INVALID_INDEX;
        return NULL;
    }
    
    Matrix* result = Matrix_Clone(m, error);
    if (!result) return NULL;
    
    char temp_sum[16];
    char temp_mul[16];
    
    for (size_t j = 0; j < m->cols; j++) {
        void* target = matrix_element_ptr(result, row_idx, j);
        
        memset(temp_sum, 0, m->type->size);
        
        for (size_t k = 0; k < m->rows; k++) {
            if (k == row_idx) continue;
            
            const void* alpha = (const char*)alphas + k * m->type->size;
            void* elem = matrix_element_ptr(m, k, j);
            
            m->type->mul(temp_mul, alpha, elem);
            m->type->add(temp_sum, temp_sum, temp_mul);
        }
        
        m->type->add(target, target, temp_sum);
    }
    
    return result;
}

Matrix* Matrix_Clone(const Matrix* m, MatrixError* error) {
    if (error) *error = MATRIX_OK;
    
    if (!m) {
        if (error) *error = MATRIX_ERROR_NULL_POINTER;
        return NULL;
    }
    
    Matrix* clone = Matrix_Create(m->rows, m->cols, m->type);
    if (!clone) {
        if (error) *error = MATRIX_ERROR_MEMORY;
        return NULL;
    }
    
    size_t total_bytes = m->rows * m->cols * m->type->size;
    memcpy(clone->data, m->data, total_bytes);
    
    return clone;
}

MatrixError Matrix_Fill(Matrix* m, const void* value) {
    if (!m || !value) return MATRIX_ERROR_NULL_POINTER;
    
    size_t total = m->rows * m->cols;
    for (size_t i = 0; i < total; i++) {
        void* elem = (char*)m->data + i * m->type->size;
        memcpy(elem, value, m->type->size);
    }
    
    return MATRIX_OK;
}

MatrixError Matrix_Identity(Matrix* m) {
    if (!m) return MATRIX_ERROR_NULL_POINTER;
    if (m->rows != m->cols) return MATRIX_ERROR_DIMENSION_MISMATCH;
    
    memset(m->data, 0, m->rows * m->cols * m->type->size);
    
    for (size_t i = 0; i < m->rows; i++) {
        void* elem = matrix_element_ptr(m, i, i);
        
        if (m->type == GetIntFieldInfo()) {
            int one = 1;
            memcpy(elem, &one, m->type->size);
        } else {
            float one = 1.0f;
            memcpy(elem, &one, m->type->size);
        }
    }
    
    return MATRIX_OK;
}

MatrixError Matrix_Print(const Matrix* m, const char* name, FILE* output) {
    if (!m || !output) return MATRIX_ERROR_NULL_POINTER;
    
    if (name) {
        fprintf(output, "%s = ", name);
    }
    
    fprintf(output, "[");
    for (size_t i = 0; i < m->rows; i++) {
        if (i > 0) fprintf(output, " ");
        fprintf(output, "[");
        for (size_t j = 0; j < m->cols; j++) {
            void* elem = matrix_element_ptr(m, i, j);
            m->type->print(elem, output);
            if (j < m->cols - 1) fprintf(output, " ");
        }
        fprintf(output, "]");
        if (i < m->rows - 1) fprintf(output, "\n");
    }
    fprintf(output, "]\n");
    
    return MATRIX_OK;
}

Matrix* Matrix_Read(FILE* input, const FieldInfo* type, MatrixError* error) {
    if (error) *error = MATRIX_OK;
    
    if (!input || !type) {
        if (error) *error = MATRIX_ERROR_NULL_POINTER;
        return NULL;
    }
    
    size_t rows, cols;
    if (fscanf(input, "%zu %zu", &rows, &cols) != 2) {
        if (error) *error = MATRIX_ERROR_INVALID_SIZE;
        return NULL;
    }
    
    Matrix* m = Matrix_Create(rows, cols, type);
    if (!m) {
        if (error) *error = MATRIX_ERROR_MEMORY;
        return NULL;
    }
    
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            void* elem = matrix_element_ptr(m, i, j);
            type->read(elem, input);
        }
    }
    
    return m;
}

const char* Matrix_ErrorString(MatrixError error) {
    switch (error) {
        case MATRIX_OK: return "Успешно";
        case MATRIX_ERROR_NULL_POINTER: return "Нулевой указатель";
        case MATRIX_ERROR_MEMORY: return "Ошибка выделения памяти";
        case MATRIX_ERROR_INVALID_SIZE: return "Некорректный размер";
        case MATRIX_ERROR_TYPE_MISMATCH: return "Несовпадение типов";
        case MATRIX_ERROR_DIMENSION_MISMATCH: return "Несовпадение размерностей";
        case MATRIX_ERROR_INVALID_INDEX: return "Индекс вне диапазона";
        case MATRIX_ERROR_SINGULAR_MATRIX: return "Вырожденная матрица"; 
        default: return "Неизвестная ошибка";
    }
}

//Метод Гаусса для решения СЛАУ
MatrixError Matrix_GaussSolve(const Matrix* a, const Matrix* b, Matrix* x) {
    if (!a || !b || !x) return MATRIX_ERROR_NULL_POINTER;
    if (!a->type || !b->type || !x->type) return MATRIX_ERROR_TYPE_MISMATCH;
    if (!types_compatible(a, b) || !types_compatible(a, x)) return MATRIX_ERROR_TYPE_MISMATCH;
    
    // Проверка: A - квадратная, b - вектор-столбец
    if (a->rows != a->cols) return MATRIX_ERROR_DIMENSION_MISMATCH;
    if (b->cols != 1) return MATRIX_ERROR_DIMENSION_MISMATCH;
    if (a->rows != b->rows) return MATRIX_ERROR_DIMENSION_MISMATCH;
    if (x->rows != a->rows || x->cols != 1) return MATRIX_ERROR_DIMENSION_MISMATCH;
    
    size_t n = a->rows;
    
     // Создаем расширенную матрицу [A|b]
    Matrix* augmented = Matrix_Create(n, n + 1, a->type);
    if (!augmented) return MATRIX_ERROR_MEMORY;
    
    // Копируем A и b (как в оригинале)
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            void* src = matrix_element_ptr(a, i, j);
            void* dst = matrix_element_ptr(augmented, i, j);
            memcpy(dst, src, a->type->size);
        }
        void* src = matrix_element_ptr(b, i, 0);
        void* dst = matrix_element_ptr(augmented, i, n);
        memcpy(dst, src, b->type->size);
    }
    
    char temp[16];
    char factor[16];
    
    // Прямой ход метода Гаусса
    for (size_t k = 0; k < n; k++) {
        // Поиск главного элемента
        size_t max_row = k;
        void* max_pivot = matrix_element_ptr(augmented, k, k);
        
        for (size_t i = k + 1; i < n; i++) {
            void* current = matrix_element_ptr(augmented, i, k);
            
            if (a->type == GetFloatFieldInfo()) {
                float max_val = *(float*)max_pivot;
                float cur_val = *(float*)current;
                if (cur_val < 0) cur_val = -cur_val;
                if (max_val < 0) max_val = -max_val;
                if (cur_val > max_val) {
                    max_row = i;
                    max_pivot = current;
                }
            } else if (a->type == GetIntFieldInfo()) {
                int max_val = *(int*)max_pivot;
                int cur_val = *(int*)current;
                if (cur_val < 0) cur_val = -cur_val;
                if (max_val < 0) max_val = -max_val;
                if (cur_val > max_val) {
                    max_row = i;
                    max_pivot = current;
                }
            }
        }
        
        // Проверка на вырожденность
        if (a->type == GetFloatFieldInfo()) {
            float pivot_abs = *(float*)max_pivot;
            if (pivot_abs < 0) pivot_abs = -pivot_abs;
            if (pivot_abs < 1e-10f) {
                Matrix_Destroy(augmented);
                return MATRIX_ERROR_SINGULAR_MATRIX;
            }
        } else if (a->type == GetIntFieldInfo()) {
            if (*(int*)max_pivot == 0) {
                Matrix_Destroy(augmented);
                return MATRIX_ERROR_SINGULAR_MATRIX;
            }
        }
        
        // Меняем строки, если нужно
        if (max_row != k) {
            for (size_t j = 0; j < n + 1; j++) {
                void* row_k = matrix_element_ptr(augmented, k, j);
                void* row_max = matrix_element_ptr(augmented, max_row, j);
                memcpy(temp, row_k, a->type->size);
                memcpy(row_k, row_max, a->type->size);
                memcpy(row_max, temp, a->type->size);
            }
        }
        
        //Исключение переменной из нижних строк
        for (size_t i = k + 1; i < n; i++) {
            void* elem_i_k = matrix_element_ptr(augmented, i, k);
            
            //Если элемент уже нулевой, пропускаем
            if (a->type == GetFloatFieldInfo()) {
                if (*(float*)elem_i_k == 0) continue;
            }
            
            memcpy(factor, elem_i_k, a->type->size);
            
            
            void* pivot = matrix_element_ptr(augmented, k, k);
            a->type->div(factor, factor, pivot);
            
            for (size_t j = k; j < n + 1; j++) {
                void* elem_i_j = matrix_element_ptr(augmented, i, j);
                void* elem_k_j = matrix_element_ptr(augmented, k, j);
                
                a->type->mul(temp, factor, elem_k_j);
                a->type->sub(elem_i_j, elem_i_j, temp);
            }
        }
    }
    
    //Обратный ход 
    for (size_t i = n; i-- > 0; ) {
        void* x_i = matrix_element_ptr(x, i, 0);
        void* b_i = matrix_element_ptr(augmented, i, n);
        memcpy(x_i, b_i, a->type->size);
        
        for (size_t j = i + 1; j < n; j++) {
            void* a_i_j = matrix_element_ptr(augmented, i, j);
            void* x_j = matrix_element_ptr(x, j, 0);
            
            a->type->mul(temp, a_i_j, x_j);
            a->type->sub(x_i, x_i, temp);
        }
        
        // Делим на диагональный элемент
        void* a_i_i = matrix_element_ptr(augmented, i, i);
        a->type->div(x_i, x_i, a_i_i);
    }
    
    Matrix_Destroy(augmented);
    return MATRIX_OK;
}
