# CubicTransition

This class is similar to [WeightedTransition.md], except that it computes a
cubic polynomial fit in the transition region instead of weighting the two
functions:
\begin{equation}
  f(x) = A x^3 + B x^2 + C x + D \,,
\end{equation}
where the polynomial coefficients are fitted to satisfy the properties listed
for [WeightedTransition.md], which gives 4 equations:

- $f(x_1) = f_1(x_1)$
- $f(x_2) = f_2(x_2)$
- $f'(x_1) = f_1'(x_1)$
- $f'(x_2) = f_2'(x_2)$

As noted for [WeightedTransition.md], this transition is recommended over
[WeightedTransition.md] when the transition occurs at the intersection of two
functions.
