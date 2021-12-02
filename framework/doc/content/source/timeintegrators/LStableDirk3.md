# LStableDirk3

!syntax description /Executioner/TimeIntegrator/LStableDirk3

This method can be expressed as a Runge-Kutta method with the following Butcher Tableau:

!equation
\begin{array}{c|ccc}
  \gamma & \gamma \\
  (1-\gamma)/2 & \gamma \\
  1 & (1/4)(-6\gamma^2 + 16\gamma - 1) & (1/4)(6\gamma^2 - 20\gamma + 5) & \gamma \\
\hline
    &  (1/4)(-6\gamma^2 + 16\gamma - 1)  & (1/4)(6\gamma^2 - 20 \gamma + 5) & \gamma
\end{array}

where $\gamma = -\sqrt{2} \cos(atan(\sqrt{2}/4)/3)/2 + \sqrt{6} \sin(atan(\sqrt{2}/4)/3)/2 + 1  \approx 0.435866521508459$

This method can be expressed as a Runge-Kutta method with the following Butcher Tableau:

!equation
R(z) = \dfrac{1.90128552647780115 z^2 + 2.46079651620301599 z - 8}{
       0.662446064957040178 z^3 - 4.55951098972521484 z^2 + 10.460796516203016 z - 8}

The method is L-stable:

!equation
\lim_{z->\infty} R(z) = 0

This method is derived in detail in [!cite](alexander1967). Unlike BDF3,
this method is L-stable and so may be more suitable for "stiff"
problems.

!syntax parameters /Executioner/TimeIntegrator/LStableDirk3

!syntax inputs /Executioner/TimeIntegrator/LStableDirk3

!syntax children /Executioner/TimeIntegrator/LStableDirk3
