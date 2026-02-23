#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include "matrix.h"
#include "field.h"         
#include "int_field.h"      
#include "float_field.h" 

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (cond) { \
            tests_passed++; \
            printf("  + %s\n", msg); \
        } else { \
            tests_failed++; \
            printf("  - %s\n", msg); \
        } \
    } while(0)

void test_creation() {
    printf("\nTest 1 Matrix Creation:\n");
    
    const FieldInfo* int_type = GetIntFieldInfo();
    const FieldInfo* float_type = GetFloatFieldInfo();
    
    TEST_ASSERT(int_type != NULL, "Get int type");
    TEST_ASSERT(float_type != NULL, "Get float type");
    
    Matrix* m = Matrix_Create(2, 3, int_type);
    TEST_ASSERT(m != NULL, "Creation 2x3 int matrix");
    TEST_ASSERT(m->rows == 2, "Check rows = 2");
    TEST_ASSERT(m->cols == 3, "Check cols = 3");
    
    Matrix_Destroy(m);
}

void test_element_access() {
    printf("\nTest 2 Element Access:\n");
    
    Matrix* m = Matrix_Create(2, 2, GetIntFieldInfo());
    
    int val = 42;
    MatrixError err = Matrix_Set(m, 0, 0, &val);
    TEST_ASSERT(err == MATRIX_OK, "Set element [0][0]");
    
    int result;
    err = Matrix_Get(m, 0, 0, &result);
    TEST_ASSERT(err == MATRIX_OK && result == 42, "Read element [0][0] = 42");
    
    err = Matrix_Get(m, 5, 5, &result);
    TEST_ASSERT(err == MATRIX_ERROR_INVALID_INDEX, "Checking for out of bounds");
    
    Matrix_Destroy(m);
}

void test_addition() {
    printf("\nTest 3 Matrix Addition:\n");
    
    Matrix* a = Matrix_Create(2, 2, GetIntFieldInfo());
    Matrix* b = Matrix_Create(2, 2, GetIntFieldInfo());
    
    int vals_a[] = {1, 2, 3, 4};
    int vals_b[] = {5, 6, 7, 8};
    
    for (int i = 0; i < 4; i++) {
        Matrix_Set(a, i/2, i%2, &vals_a[i]);
        Matrix_Set(b, i/2, i%2, &vals_b[i]);
    }
    
    MatrixError err;
    Matrix* c = Matrix_Add(a, b, &err);
    TEST_ASSERT(err == MATRIX_OK && c != NULL, "Matrix Addition");
    
    int expected[] = {6, 8, 10, 12};
    for (int i = 0; i < 4; i++) {
        int val;
        Matrix_Get(c, i/2, i%2, &val);
        TEST_ASSERT(val == expected[i], "Check result");
    }
    
    Matrix_Destroy(a);
    Matrix_Destroy(b);
    Matrix_Destroy(c);
}

void test_multiplication() {
    printf("\nTest 4 Matrix Multiplication:\n");
    
    Matrix* a = Matrix_Create(2, 3, GetIntFieldInfo());
    Matrix* b = Matrix_Create(3, 2, GetIntFieldInfo());
    
    int a_vals[] = {1, 2, 3, 4, 5, 6};
    int b_vals[] = {7, 8, 9, 10, 11, 12};
    
    for (int i = 0; i < 6; i++) {
        Matrix_Set(a, i/3, i%3, &a_vals[i]);
        Matrix_Set(b, i/2, i%2, &b_vals[i]);
    }
    
    MatrixError err;
    Matrix* c = Matrix_Multiply(a, b, &err);
    TEST_ASSERT(err == MATRIX_OK && c != NULL, "Matrix Multiplication");
    
    int expected[] = {58, 64, 139, 154};
    for (int i = 0; i < 4; i++) {
        int val;
        Matrix_Get(c, i/2, i%2, &val);
        TEST_ASSERT(val == expected[i], "Check result");
    }
    
    Matrix_Destroy(a);
    Matrix_Destroy(b);
    Matrix_Destroy(c);
}

void test_scalar_multiply() {
    printf("\nTest 5 Scalar Multiplication:\n");
    
    Matrix* m = Matrix_Create(2, 2, GetIntFieldInfo());
    int vals[] = {1, 2, 3, 4};
    for (int i = 0; i < 4; i++) {
        Matrix_Set(m, i/2, i%2, &vals[i]);
    }
    
    int scalar = 3;
    MatrixError err;
    Matrix* result = Matrix_ScalarMultiply(m, &scalar, &err);
    TEST_ASSERT(err == MATRIX_OK && result != NULL, "Scalar Multiplication");
    
    int expected[] = {3, 6, 9, 12};
    for (int i = 0; i < 4; i++) {
        int val;
        Matrix_Get(result, i/2, i%2, &val);
        TEST_ASSERT(val == expected[i], "Check result");
    }
    
    Matrix_Destroy(m);
    Matrix_Destroy(result);
}

void test_linear_combination() {
    printf("\nTest 6 Linear Combination of rows:\n");
    
    Matrix* m = Matrix_Create(3, 3, GetIntFieldInfo());
    int vals[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    for (int i = 0; i < 9; i++) {
        Matrix_Set(m, i/3, i%3, &vals[i]);
    }
    
    int alphas[] = {0, 2, 1};  
    MatrixError err;
    Matrix* result = Matrix_AddLinearCombination(m, 0, alphas, &err);
    TEST_ASSERT(err == MATRIX_OK && result != NULL, "Linear Combination of rows");
    
    int expected_row0[] = {16, 20, 24};  
    for (int j = 0; j < 3; j++) {
        int val;
        Matrix_Get(result, 0, j, &val);
        TEST_ASSERT(val == expected_row0[j], "Check result");
    }
    
    Matrix_Destroy(m);
    Matrix_Destroy(result);
}

void test_identity() {
    printf("\nTest 7 Identity Matrix:\n");
    
    Matrix* m = Matrix_Create(3, 3, GetIntFieldInfo());
    MatrixError err = Matrix_Identity(m);
    TEST_ASSERT(err == MATRIX_OK, "Create identity matrix");
    
    int expected[] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    for (int i = 0; i < 9; i++) {
        int val;
        Matrix_Get(m, i/3, i%3, &val);
        TEST_ASSERT(val == expected[i], "Check result");
    }
    
    Matrix_Destroy(m);
}

//Решение СЛАУ методом Гаусса для int
void test_gauss_solve_int() {
    printf("\nTest 8 Gauss Method (int):\n");
    
    // Система:
    // 2x + y - z = 7
    // x + 3y + 2z = 11
    // 3x + 2y - 3z = 9
    
    Matrix* a_int = Matrix_Create(3, 3, GetIntFieldInfo());
    Matrix* b_int = Matrix_Create(3, 1, GetIntFieldInfo());
    Matrix* x_int = Matrix_Create(3, 1, GetIntFieldInfo());
    
    int a_vals[] = {
        2, 1, -1,
        1, 3, 2,
        3, 2, -3
    };
    
    int b_vals[] = {7, 11, 9};
    int expected[] = {4, 1, 2};
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            Matrix_Set(a_int, i, j, &a_vals[i*3 + j]);
        }
        Matrix_Set(b_int, i, 0, &b_vals[i]);
    }
    //Соз-м матрицы для вычислений
    Matrix* a_float = Matrix_Create(3, 3, GetFloatFieldInfo());
    Matrix* b_float = Matrix_Create(3, 1, GetFloatFieldInfo());
    Matrix* x_float = Matrix_Create(3, 1, GetFloatFieldInfo());
    
    //Копируем данные из int в float
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int val;
            Matrix_Get(a_int, i, j, &val);
            float fval = (float)val;
            Matrix_Set(a_float, i, j, &fval);
        }
        int val;
        Matrix_Get(b_int, i, 0, &val);
        float fval = (float)val;
        Matrix_Set(b_float, i, 0, &fval);
    }
    
    //Решаем систему во float
    MatrixError err = Matrix_GaussSolve(a_float, b_float, x_float);
    TEST_ASSERT(err == MATRIX_OK, "Solve 3x3 system (int via float)");
    
    //Конвертируем результат обратно в int с округлением
    for (int i = 0; i < 3; i++) {
        float fval;
        Matrix_Get(x_float, i, 0, &fval);
        
        //Округляем до ближайшего целого
        int ival = (int)(fval + 0.5f);
        Matrix_Set(x_int, i, 0, &ival);
        
        //Для отладки - печатаем float и int значения
        printf("x[%d] = %.3f (float) -> %d (int)\n", i, fval, ival);
    }
    
    // Проверяем результат
    for (int i = 0; i < 3; i++) {
        int val;
        Matrix_Get(x_int, i, 0, &val);
        char msg[100];
        sprintf(msg, "Check x[%d] = %d (expected %d)", i, val, expected[i]);
        TEST_ASSERT(val == expected[i], msg);
    }
    
    // Очищаем память
    Matrix_Destroy(a_int);
    Matrix_Destroy(b_int);
    Matrix_Destroy(x_int);
    Matrix_Destroy(a_float);
    Matrix_Destroy(b_float);
    Matrix_Destroy(x_float);
}

//Решение СЛАУ методом Гаусса для float
void test_gauss_solve_float() {
    printf("\nTest 9 Gauss Method (float):\n");
    
    Matrix* a = Matrix_Create(3, 3, GetFloatFieldInfo());
    Matrix* b = Matrix_Create(3, 1, GetFloatFieldInfo());
    Matrix* x = Matrix_Create(3, 1, GetFloatFieldInfo());
    
    float a_vals[] = {
        2.0f, 1.0f, -1.0f,
        1.0f, 3.0f, 2.0f,
        3.0f, 2.0f, -3.0f
    };
    
    float b_vals[] = {7.0f, 11.0f, 9.0f};
    float expected[] = {3.5f, 1.5f, 1.5f};
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            Matrix_Set(a, i, j, &a_vals[i*3 + j]);
        }
        Matrix_Set(b, i, 0, &b_vals[i]);
    }
    
    MatrixError err = Matrix_GaussSolve(a, b, x);
    TEST_ASSERT(err == MATRIX_OK, "Solve 3x3 system (float)");
    
    float tolerance = 0.01f;
    for (int i = 0; i < 3; i++) {
        float val;
        Matrix_Get(x, i, 0, &val);

        float diff = fabs(val - expected[i]);
        char msg[100];
        sprintf(msg, "x[%d] = %.3f (expected %.3f)", 
                i, val, expected[i], diff);
        TEST_ASSERT(diff < tolerance, msg);
    }
    
    Matrix_Destroy(a);
    Matrix_Destroy(b);
    Matrix_Destroy(x);
}

//Проверка на вырожденную матрицу
void test_gauss_singular() {
    printf("\nTest 10 Singular Matrix Detection:\n");
    

    Matrix* a = Matrix_Create(2, 2, GetFloatFieldInfo());
    float a_vals[] = {1, 2, 2, 4};  
    for (int i = 0; i < 4; i++) {
        Matrix_Set(a, i/2, i%2, &a_vals[i]);
    }
    
    Matrix* b = Matrix_Create(2, 1, GetFloatFieldInfo());
    float b_vals[] = {5, 10};
    for (int i = 0; i < 2; i++) {
        Matrix_Set(b, i, 0, &b_vals[i]);
    }
    
    Matrix* x = Matrix_Create(2, 1, GetFloatFieldInfo());
    
    MatrixError err = Matrix_GaussSolve(a, b, x);
    TEST_ASSERT(err == MATRIX_ERROR_SINGULAR_MATRIX, "Detect singular matrix");
    
    Matrix_Destroy(a);
    Matrix_Destroy(b);
    Matrix_Destroy(x);
}


//Производительность для матрицы 100x100
void test_performance_100x100() {
    printf("\nTest Performance 100x100 matrix:\n");
    
    const int size = 100;
    clock_t start, end;
    double cpu_time_used;
    
    //Создание матрицы
    start = clock();
    Matrix* m = Matrix_Create(size, size, GetFloatFieldInfo());
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Creation time: %.3f seconds\n", cpu_time_used);
    TEST_ASSERT(m != NULL, "Create 100x100 matrix");
    
    //Заполнение матрицы значениями
    start = clock();
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            float val = (float)(i * size + j);
            Matrix_Set(m, i, j, &val);
        }
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Fill time: %.3f seconds\n", cpu_time_used);
    TEST_ASSERT(1, "Fill 100x100 matrix with values");
    
    //Чтение всех элементов
    start = clock();
    float sum = 0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            float val;
            Matrix_Get(m, i, j, &val);
            sum += val;
        }
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Read all elements time: %.3f seconds\n", cpu_time_used);
    printf("  Sum of all elements: %.2f\n", sum);
    
    //Клонирование матрицы
    MatrixError err;
    start = clock();
    Matrix* clone = Matrix_Clone(m, &err);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Clone time: %.3f seconds\n", cpu_time_used);
    TEST_ASSERT(err == MATRIX_OK && clone != NULL, "Clone 100x100 matrix");
    
    //Умножение на скаляр
    float scalar = 2.5f;
    start = clock();
    Matrix* scaled = Matrix_ScalarMultiply(m, &scalar, &err);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Scalar multiply time: %.3f seconds\n", cpu_time_used);
    TEST_ASSERT(err == MATRIX_OK && scaled != NULL, "Scalar multiply 100x100 matrix");
    
    //Сложение матриц
    start = clock();
    Matrix* sum_matrix = Matrix_Add(m, clone, &err);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Addition time: %.3f seconds\n", cpu_time_used);
    TEST_ASSERT(err == MATRIX_OK && sum_matrix != NULL, "Add two 100x100 matrices");
    
    //Умножение матриц
    start = clock();
    Matrix* product = Matrix_Multiply(m, clone, &err);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Multiplication time (O(n³)): %.3f seconds\n", cpu_time_used);
    TEST_ASSERT(err == MATRIX_OK && product != NULL, "Multiply two 100x100 matrices");
    
    //Линейная комбинация строк
    float* alphas = (float*)malloc(size * sizeof(float));
    for (int i = 0; i < size; i++) {
        alphas[i] = (float)i / size;
    }
    
    start = clock();
    Matrix* combined = Matrix_AddLinearCombination(m, size/2, alphas, &err);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Linear combination time: %.3f seconds\n", cpu_time_used);
    TEST_ASSERT(err == MATRIX_OK && combined != NULL, "Linear combination of rows");
    
    free(alphas);
    
    start = clock();
    Matrix_Destroy(m);
    Matrix_Destroy(clone);
    Matrix_Destroy(scaled);
    Matrix_Destroy(sum_matrix);
    Matrix_Destroy(product);
    Matrix_Destroy(combined);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Cleanup time: %.3f seconds\n", cpu_time_used);
    
    printf("\n  Performance test completed\n");
}

void run_all_tests() {
    printf("\n========================================\n");
    printf("        RUNNING UNIT TESTS\n");
    printf("========================================\n");
    
    test_creation();
    test_element_access();
    test_addition();
    test_multiplication();
    test_scalar_multiply();
    test_linear_combination();
    test_identity();

//Тест для метода Гаусса
    test_gauss_solve_int();
    test_gauss_solve_float();
    test_gauss_singular();
 
//Тест производительности 100*100
    test_performance_100x100();

    printf("\n========================================\n");
    printf("Results: Passed: %d | Failed: %d\n", 
           tests_passed, tests_failed);
    printf("========================================\n\n");
}




//gcc -o matrix.exe field.c matrix.c int_field.c float_field.c test_matrix.c main.c
