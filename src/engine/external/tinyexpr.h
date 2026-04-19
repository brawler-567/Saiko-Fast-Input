 
 
 

#ifndef TINYEXPR_H
#define TINYEXPR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef union te_function {
    void *any;
    double (*f0)(void);
    double (*f1)(double);
    double (*f2)(double,double);
    double (*f3)(double,double,double);
    double (*f4)(double,double,double,double);
    double (*f5)(double,double,double,double,double);
    double (*f6)(double,double,double,double,double,double);
    double (*f7)(double,double,double,double,double,double,double);
    double (*cl0)(void*);
    double (*cl1)(void*,double);
    double (*cl2)(void*,double,double);
    double (*cl3)(void*,double,double,double);
    double (*cl4)(void*,double,double,double,double);
    double (*cl5)(void*,double,double,double,double,double);
    double (*cl6)(void*,double,double,double,double,double,double);
    double (*cl7)(void*,double,double,double,double,double,double,double);
} te_function;

typedef union te_value {
    double value;
    const double *bound;
    te_function f;
} te_value;

typedef struct te_expr {
    int type;
    te_value v;
    struct te_expr *parameters[];
} te_expr;

enum {
    TE_VARIABLE = 0,

    TE_FUNCTION0 = 8, TE_FUNCTION1, TE_FUNCTION2, TE_FUNCTION3,
    TE_FUNCTION4, TE_FUNCTION5, TE_FUNCTION6, TE_FUNCTION7,

    TE_CLOSURE0 = 16, TE_CLOSURE1, TE_CLOSURE2, TE_CLOSURE3,
    TE_CLOSURE4, TE_CLOSURE5, TE_CLOSURE6, TE_CLOSURE7,

    TE_FLAG_PURE = 32
};

typedef struct te_variable {
    const char *name;
    const te_function address;
    int type;
    void *context;
} te_variable;

 
 
double te_interp(const char *expression, int *error);

 
 
te_expr *te_compile(const char *expression, const te_variable *variables, int var_count, int *error);

 
double te_eval(const te_expr *n);

 
void te_print(const te_expr *n);

 
 
void te_free(te_expr *n);

#ifdef __cplusplus
}
#endif

#endif  
