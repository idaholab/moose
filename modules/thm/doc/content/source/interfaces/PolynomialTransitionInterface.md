# PolynomialTransitionInterface

This interface is similar to [WeightedTransitionInterface.md], except that it
computes a cubic polynomial fit in the transition region instead of weighting
the two functions:
\begin{equation}
  f(x) = A x^3 + B x^2 + C x + D \,,
\end{equation}
where the polynomial coefficients are fitted to satisfy the properties listed
for [WeightedTransitionInterface.md], which gives 4 equations:

- $f(x_1) = f_1(x_1)$
- $f(x_2) = f_2(x_2)$
- $f'(x_1) = f_1'(x_1)$
- $f'(x_2) = f_2'(x_2)$

As noted for [WeightedTransitionInterface.md], this transition is recommended
over [WeightedTransitionInterface.md] when the transition occurs at the
intersection of two functions.
