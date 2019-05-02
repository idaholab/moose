# AdamsPredictor

## Description

`AdamsPredictor` uses the Adams-Bashforth scheme to estimate the solution at the new time step value $x_{n+1}$ by a linear combination of $x_{n}$, $x_{n-1}$, and $x_{n-2}$:

\begin{equation}
  x_{n+1} = a x_n + b x_{n-1} + c x_{n-2},
\end{equation}
where
\begin{align}
   a & = 1 + \frac{\Delta t_n}{\Delta t_{n-1}} \left( 1 + \frac{1}{2}\frac{\Delta t_n}{\Delta t_{n-1}} \right)\\
   b & = -\frac{\Delta t_n}{\Delta t_{n-1}} \left( 1 + \frac{1}{2} \frac{\Delta t_n}{\Delta t_{n-1}}
     + \frac{1}{2} \frac{\Delta t_n}{\Delta t_{n-2}} \right)\\
   c & = \frac{\Delta t_n^2}{2 \Delta t_{n-1} \Delta t_{n-2}}.
\end{align}


!syntax description /Executioner/Predictor/AdamsPredictor

!syntax parameters /Executioner/Predictor/AdamsPredictor

!syntax inputs /Executioner/Predictor/AdamsPredictor

!syntax children /Executioner/Predictor/AdamsPredictor

!bibtex bibliography
