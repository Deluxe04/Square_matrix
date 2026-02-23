#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrix.h"
#include "field.h"         
#include "int_field.h"      
#include "float_field.h" 

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/time.h>
#endif

static Matrix* current_matrix = NULL;

void print_menu() {
    printf("\n=============================================\n");
    printf("        POLYMORPHIC MATRIX\n");
    printf("=============================================\n");
    
    if (current_matrix) {
        printf("Current matrix: %zux%zu (%s)\n",
               current_matrix->rows, current_matrix->cols,
               current_matrix->type->name);
        Matrix_Print(current_matrix, "current", stdout);
    } else {
        printf("Current matrix: not created\n");
    }
    
    printf("\n");
    printf("1. Create matrix (int)\n");
    printf("2. Create matrix (float)\n");
    printf("3. Create identity matrix\n");
    printf("4. Add two matrices\n");
    printf("5. Multiply two matrices\n");
    printf("6. Multiply by scalar\n");
    printf("7. Add linear combination of rows\n");
    printf("8. Fill matrix with value\n");
    printf("9. Show current matrix\n");
    printf("10. Solve linear system (Gauss method)\n");
    printf("11. Performance test (100x100 matrix)\n");   
    printf("12. Run tests\n");     
    printf("0. Exit\n");
    printf("\nChoose action: ");
}

//Точное измерение времени
double get_precise_time_ms() {
#ifdef _WIN32
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart * 1000.0 / (double)frequency.QuadPart;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0;
#endif
}

void performance_test() {
    printf("\n========================================\n");
    printf("   PERFORMANCE TEST: 100x100 MATRIX\n");
    printf("========================================\n");
    
    const int size = 100;
    double start, end;
    double cpu_time_used;
    
    printf("\nCreating %d x %d matrix\n", size, size);
    
    //Прогрев кэша
    printf("\nWarming up cache\n");
    Matrix* warm = Matrix_Create(size, size, GetFloatFieldInfo());
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            float val = (float)j;
            Matrix_Set(warm, i, j, &val);
        }
    }
    Matrix_Destroy(warm);

    //Создание матрицы
    start = get_precise_time_ms();
    Matrix* m = Matrix_Create(size, size, GetFloatFieldInfo());
    end = get_precise_time_ms();
    cpu_time_used = end - start;
    printf("1. Creation: %.2f ms\n", cpu_time_used);
    
    if (!m) {
        printf("Failed to create matrix!\n");
        return;
    }
    
    //Заполнение матрицы
    start = get_precise_time_ms();
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            float val = (float)(i * size + j);
            Matrix_Set(m, i, j, &val);
        }
    }
    end = get_precise_time_ms();
    cpu_time_used = end - start;
    printf("2. Fill %d elements: %.2f ms (%.3f ms per element)\n", 
           size * size, cpu_time_used, cpu_time_used / (size * size));
    
    //Чтение матрицы
    start = get_precise_time_ms();
    float sum = 0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            float val;
            Matrix_Get(m, i, j, &val);
            sum += val;
        }
    }
    end = get_precise_time_ms();
    cpu_time_used = end - start;
    printf("3. Read all elements: %.2f ms (sum = %.2f)\n", cpu_time_used, sum);
    
    // 4. Клонирование
    MatrixError err;
    start = get_precise_time_ms();
    Matrix* clone = Matrix_Clone(m, &err);
    end = get_precise_time_ms();
    cpu_time_used = end - start;
    printf("4. Clone matrix: %.2f ms\n", cpu_time_used);
    
    //Умножение на скаляр
    float scalar = 2.5f;
    start = get_precise_time_ms();
    Matrix* scaled = Matrix_ScalarMultiply(m, &scalar, &err);
    end = get_precise_time_ms();
    cpu_time_used = end - start;
    printf("5. Scalar multiply: %.2f ms\n", cpu_time_used);
    
    //Сложение матриц
    start = get_precise_time_ms();
    Matrix* sum_mat = Matrix_Add(m, clone, &err);
    end = get_precise_time_ms();
    cpu_time_used = end - start;
    printf("6. Matrix addition: %.2f ms\n", cpu_time_used);
    
    //Умножение матриц 
    printf("\n   Multiplying %d x %d matrices\n", size, size);
    
    //Делаем 3 замера
    double total_time = 0;
    int repetitions = 3;
    
    for (int r = 0; r < repetitions; r++) {
        //Создаем временную матрицу для каждого замера
        Matrix* temp = Matrix_Clone(m, &err);
        
        start = get_precise_time_ms();
        Matrix* product = Matrix_Multiply(temp, clone, &err);
        end = get_precise_time_ms();
        cpu_time_used = end - start;
        total_time += cpu_time_used;
        
        if (product) Matrix_Destroy(product);
        Matrix_Destroy(temp);
        
        printf("Run %d: %.2f ms\n", r + 1, cpu_time_used);
    }
    
    double avg_time = total_time / repetitions;
    printf("7. Matrix multiplication (avg of %d runs): %.2f ms (%.3f seconds)\n", 
           repetitions, avg_time, avg_time / 1000.0);
    
    //Очистка памяти
    start = get_precise_time_ms();
    Matrix_Destroy(m);
    Matrix_Destroy(clone);
    Matrix_Destroy(scaled);
    Matrix_Destroy(sum_mat);
    end = get_precise_time_ms();
    cpu_time_used = end - start;
    printf("8. Cleanup: %.2f ms\n", cpu_time_used);
    
    printf("\n========================================\n");
    printf("        PERFORMANCE TEST COMPLETE\n");
    printf("========================================\n");
}

void solve_linear_system() {
    if (!current_matrix) {
        printf("\nError: no current matrix!\n");
        return;
    }
    
    //Проверяем, что текущая матрица - квадратная
    if (current_matrix->rows != current_matrix->cols) {
        printf("Error: matrix must be square for solving linear system!\n");
        return;
    }
    
    printf("\nSolving linear system A * x = b\n");
    printf("Matrix A (coefficients):\n");
    Matrix_Print(current_matrix, "A", stdout);
    
    //Создаем вектор правой части b
    Matrix* b = Matrix_Create(current_matrix->rows, 1, current_matrix->type);
    if (!b) {
        printf("Error: failed to create vector b!\n");
        return;
    }
    
    printf("\nEnter right-hand side vector b (%zu elements):\n", current_matrix->rows);
    for (size_t i = 0; i < current_matrix->rows; i++) {
        if (current_matrix->type == GetIntFieldInfo()) {
            int val;
            printf("b[%zu] = ", i);
            scanf("%d", &val);
            Matrix_Set(b, i, 0, &val);
        } else {
            float val;
            printf("b[%zu] = ", i);
            scanf("%f", &val);
            Matrix_Set(b, i, 0, &val);
        }
    }
    
    printf("\nVector b:\n");
    Matrix_Print(b, "b", stdout);
    
    //Создаем вектор для решения x
    Matrix* x = Matrix_Create(current_matrix->rows, 1, current_matrix->type);
    if (!x) {
        printf("Error: failed to create solution vector!\n");
        Matrix_Destroy(b);
        return;
    }
    
    MatrixError err = Matrix_GaussSolve(current_matrix, b, x);
    
    if (err == MATRIX_OK) {
        printf("\nSolution x:\n");
        Matrix_Print(x, "x", stdout); 
        
        //Проверка: A * x должно равняться b
        Matrix* check = Matrix_Multiply(current_matrix, x, &err);
        if (err == MATRIX_OK && check) {
            printf("\nVerification A * x:\n");
            Matrix_Print(check, "A*x", stdout);
            Matrix_Destroy(check);
        }
    } else {
        printf("Error solving system: %s\n", Matrix_ErrorString(err));
        
        if (err == MATRIX_ERROR_SINGULAR_MATRIX) {
            printf("The matrix is singular (determinant = 0).\n");
            printf("The system has either no solution or infinitely many solutions.\n");
        }
    }
    
    Matrix_Destroy(b);
    Matrix_Destroy(x);
}

void create_matrix(int type_choice) {   
    int rows, cols;
    printf("\nEnter number of rows and columns: ");
    scanf("%d %d", &rows, &cols);
    
    if (rows <= 0 || cols <= 0) {
        printf("Error: dimensions must be positive!\n");
        return;
    }
    
    const FieldInfo* type = (type_choice == 1) ? GetIntFieldInfo() : GetFloatFieldInfo();
    
    Matrix* new_matrix = Matrix_Create(rows, cols, type);
    if (!new_matrix) {
        printf("Error: failed to create matrix!\n");
        return;
    }
    
    printf("Enter matrix elements (%d elements):\n", rows * cols);
    for (int i = 0; i < rows; i++) {
        printf("Row %d: ", i);
        for (int j = 0; j < cols; j++) {
            if (type == GetIntFieldInfo()) {
                int val;
                scanf("%d", &val);
                Matrix_Set(new_matrix, i, j, &val);
            } else {
                float val;
                scanf("%f", &val);
                Matrix_Set(new_matrix, i, j, &val);
            }
        }
    }
    
    if (current_matrix) Matrix_Destroy(current_matrix);
    current_matrix = new_matrix;
    
    printf("Matrix created successfully!\n");
}

void create_identity() {
    int size;
    printf("\nEnter identity matrix size: ");
    scanf("%d", &size);
    
    if (size <= 0) {
        printf("Error: size must be positive!\n");
        return;
    }
    
    int type_choice;
    printf("Type (0 - int, 1 - float): ");
    scanf("%d", &type_choice);
    
    const FieldInfo* type = (type_choice == 0) ? GetIntFieldInfo() : GetFloatFieldInfo();
    
    Matrix* new_matrix = Matrix_Create(size, size, type);
    if (!new_matrix) {
        printf("Error: failed to create matrix!\n");
        return;
    }
    
    MatrixError err = Matrix_Identity(new_matrix);
    if (err != MATRIX_OK) {
        printf("Error: %s\n", Matrix_ErrorString(err));
        Matrix_Destroy(new_matrix);
        return;
    }
    
    if (current_matrix) Matrix_Destroy(current_matrix);
    current_matrix = new_matrix;
    
    printf("Identity matrix created successfully!\n");
}

void add_matrices() {
    if (!current_matrix) {
        printf("\nError: create first matrix first!\n");
        return;
    }
    
    printf("\nCreating second matrix of same size (%zux%zu)\n", 
           current_matrix->rows, current_matrix->cols);
    
    Matrix* m2 = Matrix_Create(current_matrix->rows, current_matrix->cols, current_matrix->type);
    if (!m2) {
        printf("Error: failed to create second matrix!\n");
        return;
    }
    
    printf("Enter second matrix elements:\n");
    for (size_t i = 0; i < current_matrix->rows; i++) {
        printf("Row %zu: ", i);
        for (size_t j = 0; j < current_matrix->cols; j++) {
            if (current_matrix->type == GetIntFieldInfo()) {
                int val;
                scanf("%d", &val);
                Matrix_Set(m2, i, j, &val);
            } else {
                float val;
                scanf("%f", &val);
                Matrix_Set(m2, i, j, &val);
            }
        }
    }
    
    printf("\nMatrix A:\n");
    Matrix_Print(current_matrix, "A", stdout);
    printf("\nMatrix B:\n");
    Matrix_Print(m2, "B", stdout);
    
    MatrixError err;
    Matrix* result = Matrix_Add(current_matrix, m2, &err);
    
    if (err == MATRIX_OK && result) {
        printf("\nResult A + B:\n");
        Matrix_Print(result, "C", stdout);
        
        printf("\nSave result? (1-yes/0-no): ");
        int save;
        scanf("%d", &save);
        
        if (save) {
            Matrix_Destroy(current_matrix);
            current_matrix = result;
            printf("Result saved\n");
        } else {
            Matrix_Destroy(result);
        }
    } else {
        printf("Error: %s\n", Matrix_ErrorString(err));
    }
    
    Matrix_Destroy(m2);
}

void multiply_matrices() {
    if (!current_matrix) {
        printf("\nError: create first matrix first!\n");
        return;
    }
    
    int cols2;
    printf("\nEnter number of columns for second matrix: ");
    scanf("%d", &cols2);
    
    if (cols2 <= 0) {
        printf("Error: invalid number of columns\n");
        return;
    }
    
    Matrix* m2 = Matrix_Create(current_matrix->cols, cols2, current_matrix->type);
    if (!m2) {
        printf("Error: failed to create second matrix!\n");
        return;
    }
    
    printf("Enter second matrix elements (%zux%d):\n", current_matrix->cols, cols2);
    for (size_t i = 0; i < current_matrix->cols; i++) {
        printf("Row %zu: ", i);
        for (int j = 0; j < cols2; j++) {
            if (current_matrix->type == GetIntFieldInfo()) {
                int val;
                scanf("%d", &val);
                Matrix_Set(m2, i, j, &val);
            } else {
                float val;
                scanf("%f", &val);
                Matrix_Set(m2, i, j, &val);
            }
        }
    }
    
    printf("\nMatrix A (%zux%zu):\n", current_matrix->rows, current_matrix->cols);
    Matrix_Print(current_matrix, "A", stdout);
    printf("\nMatrix B (%zux%d):\n", current_matrix->cols, cols2);
    Matrix_Print(m2, "B", stdout);
 
    MatrixError err;
    Matrix* result = Matrix_Multiply(current_matrix, m2, &err);
    
    if (err == MATRIX_OK && result) {
        printf("\nResult A × B:\n");
        Matrix_Print(result, "C", stdout);
        
        printf("\nSave result? (1-yes/0-no): ");
        int save;
        scanf("%d", &save);
        
        if (save) {
            Matrix_Destroy(current_matrix);
            current_matrix = result;
            printf("Result saved\n");
        } else {
            Matrix_Destroy(result);
        }
    } else {
        printf("Error: %s\n", Matrix_ErrorString(err));
    }
    
    Matrix_Destroy(m2);
}

void scalar_multiply() {
    if (!current_matrix) {
        printf("\nError: no current matrix!\n");
        return;
    }
    
    printf("\nCurrent matrix:\n");
    Matrix_Print(current_matrix, "A", stdout);
    
    printf("\nEnter scalar: ");
    
    char scalar[16];
    if (current_matrix->type == GetIntFieldInfo()) {
        int val;
        scanf("%d", &val);
        memcpy(scalar, &val, sizeof(int));
    } else {
        float val;
        scanf("%f", &val);
        memcpy(scalar, &val, sizeof(float));
    }
    
    MatrixError err;
    Matrix* result = Matrix_ScalarMultiply(current_matrix, scalar, &err);
    
    if (err == MATRIX_OK && result) {
        printf("\nResult:\n");
        Matrix_Print(result, "B", stdout);
        
        printf("\nSave result? (1-yes/0-no): ");
        int save;
        scanf("%d", &save);
        
        if (save) {
            Matrix_Destroy(current_matrix);
            current_matrix = result;
            printf("Result saved\n");
        } else {
            Matrix_Destroy(result);
        }
    } else {
        printf("Error: %s\n", Matrix_ErrorString(err));
    }
}

void linear_combination() {
    if (!current_matrix) {
        printf("\nError: no current matrix!\n");
        return;
    }
    
    printf("\nCurrent matrix:\n");
    Matrix_Print(current_matrix, "A", stdout);
    
    size_t row_idx;
    printf("\nEnter row index to modify (0-%zu): ", 
           current_matrix->rows - 1);
    scanf("%zu", &row_idx);
    
    if (row_idx >= current_matrix->rows) {
        printf("Error: invalid row index!\n");
        return;
    }
    
    char* alphas = malloc(current_matrix->rows * current_matrix->type->size); 
    
    printf("Enter coefficients for each row:\n");
    for (size_t i = 0; i < current_matrix->rows; i++) {
        if (i == row_idx) {
            if (current_matrix->type == GetIntFieldInfo()) {
                int zero = 0;
                memcpy(alphas + i * current_matrix->type->size, &zero, 
                       current_matrix->type->size);
            } else {
                float zero = 0.0f;
                memcpy(alphas + i * current_matrix->type->size, &zero, 
                       current_matrix->type->size);
            }
            continue;
        }
        
        printf("alpha[%zu] = ", i);
        if (current_matrix->type == GetIntFieldInfo()) {
            int val;
            scanf("%d", &val);
            memcpy(alphas + i * current_matrix->type->size, &val, 
                   current_matrix->type->size);
        } else {
            float val;
            scanf("%f", &val);
            memcpy(alphas + i * current_matrix->type->size, &val, 
                   current_matrix->type->size);
        }
    }
    
    MatrixError err;
    Matrix* result = Matrix_AddLinearCombination(current_matrix, (int)row_idx, alphas, &err);
    
    if (err == MATRIX_OK && result) {
        printf("\nResult:\n");
        Matrix_Print(result, "B", stdout);
        
        printf("\nSave result? (1-yes/0-no): ");
        int save;
        scanf("%d", &save);
        
        if (save) {
            Matrix_Destroy(current_matrix);
            current_matrix = result;
            printf("Result saved\n");
        } else {
            Matrix_Destroy(result);
        }
    } else {
        printf("Error: %s\n", Matrix_ErrorString(err));
    }
    
    free(alphas);
}

void fill_matrix() {
    if (!current_matrix) {
        printf("\nError: no current matrix!\n");
        return;
    }
    
    printf("\nCurrent matrix:\n");
    Matrix_Print(current_matrix, "A", stdout);
    
    printf("\nEnter value to fill: ");
    
    char value[16];
    if (current_matrix->type == GetIntFieldInfo()) {
        int val;
        scanf("%d", &val);
        memcpy(value, &val, sizeof(int));
    } else {
        float val;
        scanf("%f", &val);
        memcpy(value, &val, sizeof(float));
    }
    
    MatrixError err = Matrix_Fill(current_matrix, value);
    
    if (err == MATRIX_OK) {
        printf("\nMatrix filled:\n");
        Matrix_Print(current_matrix, "A", stdout);
    } else {
        printf("Error: %s\n", Matrix_ErrorString(err));
    }
}

void show_matrix() {
    if (current_matrix) {
        printf("\n");
        Matrix_Print(current_matrix, "Current matrix", stdout);
    } else {
        printf("\nNo matrix created\n");
    }
}

void run_tests() {
    extern void run_all_tests(void);
    run_all_tests();
}


//Функция для очистки буфера ввода
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

//Функция для безопасного чтения целого числа
int safe_read_int() {
    int val;
    char c;
    
    while (1) {
        if (scanf("%d", &val) == 1) {
            //Проверяем, что после него нет мусора
            c = getchar();
            if (c == '\n' || c == EOF) {
                return val;  
            } else {
                printf("Error: please enter only a number (no extra characters)!\n");
                clear_input_buffer();
            }
        } else {
            printf("Error: invalid input! Please enter a number.\n");
            clear_input_buffer();
        }
        printf("Enter your choice: ");
    }
}

int main() {   
    int choice;
    
    do {
        print_menu();
        choice = safe_read_int(); 
        printf("\n");
        
        switch (choice) {
            case 1: create_matrix(1); break;
            case 2: create_matrix(2); break;
            case 3: create_identity(); break;
            case 4: add_matrices(); break;
            case 5: multiply_matrices(); break;
            case 6: scalar_multiply(); break;
            case 7: linear_combination(); break;
            case 8:  fill_matrix(); break;
            case 9: show_matrix(); break;
            case 10:solve_linear_system(); break;
            case 11: performance_test(); break; 
            case 12: run_tests(); break;              
            case 0:  
                printf("Goodbye!\n");
                break;
            default:
                printf("Invalid choice!\n");
        }
        
        if (choice != 0) {
            printf("\n---------------------------------------------\n");
        }
    } while (choice != 0);
    
    if (current_matrix) Matrix_Destroy(current_matrix);
    return 0;
}