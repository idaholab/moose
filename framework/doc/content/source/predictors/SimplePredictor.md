# SimplePredictor

## Description

The `SimplePredictor` estimates the solution at the new time step value $x_{n+1}$ by
linearly extrapolating from the solutions at times $x_{n}$ and $x_{n-1}$:

\begin{equation}
  x_{n+1} = x_n + s \Delta t_n \frac{x_n - x_{n-1}}{\Delta t_{n-1}},
\end{equation}
where $0 \le s \le 1$ is a scaling factor, and $\Delta t_n$ is the time step size from $n$ to $n+1$.

!syntax description /Executioner/Predictor/SimplePredictor

!syntax parameters /Executioner/Predictor/SimplePredictor

!syntax inputs /Executioner/Predictor/SimplePredictor

!syntax children /Executioner/Predictor/SimplePredictor

!bibtex bibliography
