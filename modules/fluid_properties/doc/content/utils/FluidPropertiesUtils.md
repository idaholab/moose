# Utilities for the Fluid Properties Module

## Newton method solver

The fluid properties module includes a 1D and a 2D Newton method solver. 

### 1D solver

The 1D Newton method solver is used to solve the problem:

- Find $z$ such that $f(x, z) = y$, with $x$, $y$ known constants and $f$ a function. 


$f$ should compute both its return value and the derivatives of its value with regards to
both its variables using references.

The routine will error if a NaN is encountered or if the maximum number of iterations is exceeded.

### 2D solver

The 2D Newton method solver is used to solve the problem:

\begin{equation}
\begin{split}
f1(x, y) &= a \\
f2(x, y) &= b
\end{split}
\end{equation}

with $a$, $b$ known constants, $f1$, $f2$ functions and $x$, $y$ the real-valued variables to solve for.


The functions should compute both their return value and the derivatives of their value with regards to
both variables using references.

The routine will +not+ error if a NaN is encountered or if the maximum number of iterations is exceeded.
Instead, a `converged` boolean will be set to false, and the routine will return the last valid
estimates of $x$ and $y$. It is the responsibility of the user to handle this unconverged output.

## Brent's method solver

Brent's method may be used to find roots of a general function.
It was adapted from `Numerical Recipes in C++`.

More information may be found [here](https://en.wikipedia.org/wiki/Brent%27s_method).