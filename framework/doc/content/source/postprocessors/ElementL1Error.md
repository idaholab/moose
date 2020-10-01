# ElementL1Error

!syntax description /Postprocessors/ElementL1Error

This post-processor computes the following $L^1$ error norm between an elemental
variable and a function:
\begin{equation}
  \|y_h - y\|_1 = \sum\limits_i \int\limits_{\Omega_i} |y_{h,i} - y(x)| d\Omega
\end{equation}
where

- $y_h$ is the approximate solution represented by the elemental variable,
- $y$ is the reference solution represented by a function,
- $i$ is an element index, and
- $\Omega$ is a spatial domain.

!syntax parameters /Postprocessors/ElementL1Error

!syntax inputs /Postprocessors/ElementL1Error

!syntax children /Postprocessors/ElementL1Error

!bibtex bibliography
