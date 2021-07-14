# FVInfiniteCylinderRadiativeBC

!syntax description /FVBCs/FVInfiniteCylinderRadiativeBC

## Overview

This object implements a boundary flux of the form
\begin{equation}
F = \sigma c \left(T^4 - T_{\infty}^4\right)
\end{equation}

where $\sigma$ is the Stefan-Boltzmann constant, $T$ is the temperature,
$T_{\infty}$ is the temperature at infinity and the coefficient $c$ is given by

\begin{equation}
\frac{\epsilon_b \epsilon_c r_c}{\epsilon_c r_c + \epsilon_b r_b \left(1 -
\epsilon_c\right)}
\end{equation}

where $\epsilon_b$ is the emissivity of the boundary we are on (e.g. the
`boundary` parameter), $\epsilon_c$ is the emissivity of the theoretical
cylinder surrounding the theoretical cylinder bounded by our `boundary`, $r_c$
is the radius of the surrounding cylinder, and $r_b$ is the radius corresponding
to the location of our `boundary`.

!syntax parameters /FVBCs/FVInfiniteCylinderRadiativeBC

!syntax inputs /FVBCs/FVInfiniteCylinderRadiativeBC

!syntax children /FVBCs/FVInfiniteCylinderRadiativeBC
