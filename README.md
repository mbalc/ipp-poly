# ipp-poly

The following text is cited from a README prepared by [Piotr Styczyński](https://github.com/styczynski "His GitHub") to his implementation of that task, and is an effect of his sole effort. Thanks for sharing this translation and explanation!


# Exercise

This project is based on exercise we had to do on `Warsaw University, Poland`
on the `Individual Programistic Project` subject.

### Task

The exercise was to design and implement:
* Provide dynamically-allocated lists implementation - *pre-exercise*

* Polynomial handling library in pure C99 - *part no. 1*

* Interactive calculator for that library - *part no. 2*

* Implement additional functionality and provide basic unit tests - *part no. 3*

### Project structure

The project provides:
* library `src/poly.h` for handling polynomials

* additional helper libraries like dynamically allocated arrays etc.

* interactive calculator `src/calc_poly.c`

* unit tests in `tests/`

# Building

To build do the following:

* Create release folder - `mkdir release`

* Go to that folder - `cd release`

* Generate makefiles - `cmake ..`

* Build everything - `make`

* Optionally run tests - `make test`

* Optionally generate documentation (needs Doxygen to be installed) - `make doc`

* Optionally clean everything - `make clean`

# Polynomial library

All the documentation is provided in header files.

There you've got example usage of the library:

```c
  #include "poly.h"

  int main(void) {

    Poly p = PolyC(4);
    // Create constant polynomial
    // p(x) = 4

    Poly q = PolyP( PolyC(1), 2 );
    // Create polynomial that consists of
    // variable square multiplied by
    // coefficient 1
    // q(x) = 1*x^2

    // As you may see the coefficients that are placed
    // next to variables may be other polynomials
    // (polynomials from other variable)

    Poly r = PolyP( PolyP( PolyC(1), 2 ), 3 );
    // In this example
    // r(x,y) = (1*y^2)*(x^3)

    // Now you can do arithemtic operations on them
    // for example addition:
    Poly sum = PolyAdd(&p, &q);
    // sum(x) = 4 + x^2
    // If you are not sure what would happen if we sum
    // q and r then we say that the result is
    //
    // q(x) + r(x,y) = 1*x^2 + (1*y^2)*(x^3)
    // that is:
    //   x^2 + (y^2)(x^3)
    //

    // Or evalue at specific point:
    Poly rAtTwo = PolyAt(&r, 2);
    // Now we assume the first variable is equal to 2
    // So we calculate r(x,y) for x->2
    // rAtTwo = r(2,y) = (2^3)*y^2 = 8*y^2

    // You can print them nicely:
    PolyPrint(&rAtTwo);
    // ^ Do it yourself and see what the output is

    // After everything we must free them all
    PolyDestroy(&p);
    PolyDestroy(&q);
    PolyDestroy(&r);
    PolyDestroy(&sum);
    PolyDestroy(&rAtTwo);

  }
```

# Polynomial calculator

The calulator resides in `src/calc_poly.c`.

  It reads data from stdin line by line and:

1. If the line starts with lower/upper case letter then it's assumed<br>to be an input command.

2. Otherwise the input is parsed as a polynomial<br>The format of the polynomial is<br>`(COEFF_1,EXP_1)+(COEFF_2,EXP_2)+...+(COEFF_N,EXP_N)`<br>Where pair `(COEFF_N,EXP_N)` represents single monomial `(COEFF_N)*x^(EXP_N)`<br>The coefficients can be numbers:<br>`(1,3)+(2,4)` -> `x^3 + 2x^4`<br>Or polynomials (in this case they depend on other variable):<br>`((1,3)+(2,4),2)+(1,3)` -> `(x^3 + 2x^4)*y^2 + y^3`<br>can also contain monomials with the same exponent:<br>`(1,2)+(1,2)` -> 2x^2<br>Or can be just numbers:<br>`42` -> constant polynomial 42


All the polynomials are parsed and placed on top of the stack.
Then you can call one or more of the given operations

to modify the stack contents:

| Command    | Parameters | Required stack size | Description |
|------------|------------|---------------------|-------------|
|`ZERO`    |            |          0          | Pushes zero const polynomial onto stack  |
|`IS_COEFF`|            |          1          | Checks if the stack top is a constant polynomial?  |
|`IS_ZERO` |            |          1          | Checks if the stack top is a polynomial equal to 0?  |
|`CLONE`   |            |          1          | Copies the polynomial on top of the stack and places it on the top. |
|`ADD`     |            |          2          | Adds two polynomials from the top of the stack.<br>Then puts the result on the stack top. |
|`MUL`     |            |          2          | Multiplies two polynomials from the top of the stack.<br>Then puts the result on the stack top.  |
|`NEG`     |            |          1          | Changes the sign of the polynomial on the top of the stack.  |
|`SUB`     |            |          2          | Substracts two polynomials from the top of the stack.<br>Then puts the result on the stack top. |
|`IS_EQ`   |            |          2          | Checks if two top-most polynomials are equal.  |
|`DEG`     |            |          1          | Checks the degree of the top-most polynomial.<br>Degree of the polynomial is a highest exponent of variable<br>encountered in the polynomial (assuming all variables are the<br>same one).<br>E.g. degree(x^2 + yx^2) = 3  |
|`DEG_BY`  |   *index*  |          1          | Checks the degree of the top-most polynomial<br>with respect to the given variable. Variables are indexed from 0<br>(0 means the main variable of polynomial).<br><br>E.g.<br>`DegBy( ((1,2),3), 0 ) = DegBy( x^2 * y^3, 0 ) = 3`<br>`DegBy( ((1,2),3), 1 ) = DegBy( x^2 * y^3, 1 ) = 2` |
|`AT`      |   *value*  |          1          | Evaluates the top-most polynomial in specified point<br>and put it into stack.<br>The polynomial is evalued for `MAIN_VARIABLE=[value]` where<br>`MAIN_VARIABLE` is variable with index 0.<br><br>E.g.<br>`At( ((1,2),3), 0 ) = At( x^2 * y^3, 0 ) = 0`<br>`At( (1,2), 2 ) = At( x^2, 2 ) = 4`<br><br>The result of this command is always a polynomial of degree one smaller<br>than the polynomial given. |
|`PRINT`   |            |          1          | Prints the top-most polynomial. |
|`POP`     |            |          1          | Pops the top-most polynomial from the stack. |
|`POW`     |   *exp*    |          1          | Calculates the top-most polynomial power and push into the<br>stack. Obviously *exp* can be only a number! |
|`COMPOSE` |  *count*   |       *count*+1     | Takes top-most polynomail from the stack.<br>(We will call it P)<br>Then take *count* polynomials from the stack (let's call them Q1, Q2 ...).<br>Then we know that `P = C_1*x_1^E_1 + C_2*x_2^E_2 + ...`<br>so we substitute<br>`x_1 -> Q1`<br>`x_2 -> Q2`<br>etc.<br>if the `x_n` has got no matching `QN` then we assume `x_n -> 0`<br><br>Then we put result of such substitution onto the stack. |
|`DUMP`    |            |          0          | Prints the stack contents. |
|`CLEAN`   |            |          0          | Clears the stack entinerely.  |
|`EXIT`    |            |          0          | Force exits the calculator. |



