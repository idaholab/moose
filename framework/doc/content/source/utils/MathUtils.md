# MathUtils Namespace

MOOSE includes a number of C++ utility classes and functions that may be useful for developing
applications with mathematical expressions.

## Polynomial evaluations

`MathUtils::poly` evaluates a polynomial for any integer order $n>0$ using the Horner's method of evaluation,
\begin{equation}
  p(x) = c_0 + c_1 x + c_2 x^2 + ... + a_n x^n
\end{equation}
\begin{equation}
  p(x) = c_0 + x * (a_1 + x(a_2 + ... + x (a_{n-1} + x a_n)))
\end{equation}

## Clamp

`MathUtils::clamp` returns a clamped value $y$ between an upper and lower bound, $L_{lower}$ and $L_{higher}$ respectively,
\begin{equation}
  y =
  \begin{cases}
    L_{lower}  & x < L_{lower}  \\
    L_{higher} & x > L_{higher} \\
    x & \text{Otherwise}
  \end{cases}
\end{equation}

## SmootherStep

`MathUtils::smootherStep` returns a smoothed step transition between a starting and ending bounds, $B_{lower}$ and $B_{higher}$ respectively, for a given value $u$,
\begin{equation}
  x = \frac{u - B_{lower}}{B_{higher} - B_{lower}}
\end{equation}
\begin{equation}
  y =
  \begin{cases}
    0 & u <= B_{lower}  \\
    1 & u >= B_{higher} \\
    6x^5 - 15 x^4 + 10x^3 & \text{Otherwise}
  \end{cases}
\end{equation}
This method ensures a smooth transition from 0 to 1 between the two bounds, while also ensuring the first and second derivatives are zero at the two bounds.
Use of this method is especially useful when transitioning between two non-smooth regimes.
The derivative with respect to the passed value $u$ is returned using the optional derivative bool. Note that if $u = B_{lower} = B_{higher}$, then zero will be returned.
