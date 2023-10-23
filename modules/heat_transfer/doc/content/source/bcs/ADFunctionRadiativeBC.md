# ADFunctionRadiativeBC

!syntax description /BCs/ADFunctionRadiativeBC

This boundary condition computes the radiative heat flux from a boundary where the
emissivity function is provided through a Function.

\begin{equation}
 \sigma F_e (T_\infty^4-T_s^4)
\end{equation}
where $\sigma$ is the Stephan-Boltzmann constant, $F_e$ is the emissivity function,
$T_\infty$ is the temperature far from the surface, and $T_s$ is the temperature of
the surface.

!listing radiative_bcs/ad_function_radiative_bc.i block=BCs/bot_right

!syntax parameters /BCs/ADFunctionRadiativeBC

!syntax inputs /BCs/ADFunctionRadiativeBC

!syntax children /BCs/ADFunctionRadiativeBC
