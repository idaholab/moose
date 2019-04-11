# MathUtils Namespace

[MOOSE] includes a number of C++ utility classes and functions that may be useful for developing
applications with mathematical expressions.

### Polynomial evaluations
`MathUtils::poly` evaluates a polynomial for any integer order $n>0$ using the Horner's method of evaluation,
\begin{equation}
  p(x) = c_0 + c_1 x + c_2 x^2 + ... + a_n x^n
\end{equation}
\begin{equation}
  p(x) = c_0 + x * (a_1 + x(a_2 + ... + x (a_{n-1} + x a_n)))
\end{equation}

The derivative is optionally returned via the `derivative parameter`.
