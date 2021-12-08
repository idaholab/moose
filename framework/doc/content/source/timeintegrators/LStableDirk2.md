# LStableDirk2

!syntax description /Executioner/TimeIntegrator/LStableDirk2

This method can be expressed as a Runge-Kutta method with the following Butcher Tableau:

!equation
\begin{array}{c|cc}
  \alpha & \alpha\\
1 & 1 - \alpha & \alpha \\
\hline
    &  1 - \alpha  & \alpha
\end{array}

where $\alpha = 1 - \sqrt{2}/2 \approx 0.29289$

The stability function for this method is:

!equation
R(z) = 4 \dfrac{-z(-\sqrt{2} + 2) + z + 1}{z^2(-\sqrt{2} + 2)^2 - 4z(-\sqrt{2} + 2) + 4}

The method is L-stable:

!equation
\lim_{z->\infty} R(z) = 0

## Notes

This method is derived in detail in [!cite](alexander1967). This method is
more expensive than Crank-Nicolson, but has the advantage of being
L-stable (the same type of stability as the implicit Euler method)
so may be more suitable for "stiff" problems.

!syntax parameters /Executioner/TimeIntegrator/LStableDirk2

!syntax inputs /Executioner/TimeIntegrator/LStableDirk2

!syntax children /Executioner/TimeIntegrator/LStableDirk2
