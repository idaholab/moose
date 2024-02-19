# FunctorErgunDragCoefficients

!syntax description /FunctorMaterials/FunctorErgunDragCoefficients

This class implements the Darcy and Forchheimer coefficients for the Ergun drag
force model which is discussed in detail in
[PINSFVMomentumFriction.md#friction_example]. We also give details on the
definition of Darcy and Forchheimer coefficients there.

To summarize, this class implements the Darcy coefficient as

\begin{equation}
\frac{150}{D_h^2}
\end{equation}

where $D_h$ is the hydraulic diameter defined as $\frac{\epsilon d_p}{1 - \epsilon}$
where $d_p$ is the diameter of the pebbles in the bed. The Forchheimer
coefficient is given as

\begin{equation}
\frac{2 \cdot 1.75}{D_h}
\end{equation}

where we have made the $2(1.75)$ multiplication explicit, instead of writing
$3.5$, to make the 1.75 factor from the
[Ergun wikipedia page](https://en.wikipedia.org/wiki/Ergun_equation) more
recognizable.

!syntax parameters /FunctorMaterials/FunctorErgunDragCoefficients

!syntax inputs /FunctorMaterials/FunctorErgunDragCoefficients

!syntax children /FunctorMaterials/FunctorErgunDragCoefficients
