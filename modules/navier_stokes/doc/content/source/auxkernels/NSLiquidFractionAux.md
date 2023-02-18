# NSLiquidFractionAux

!syntax description /AuxKernels/NSLiquidFractionAux

The liquid fraction is defined as follows:

\begin{equation}
f_l =
\begin{cases}
f_l = 0, \text{ if } T < T_{solidus}, \\
f_l = \frac{T - T_{solidus}}{T_{liquidus} - T_{solidus},}, \text{ if } T \in (T_{solidus}, T_{liquidus}), \\
f_l = 1, \text{ if } T > T_{liquidus}, \\
\end{cases}
\end{equation}

where $T_{liquidus}$ and $T_{solidus}$ are the liquidus and solidus temperatures respectively;
note that $T_{solidus}$ must be less than $T_{liquidus}$.

The liquidus and solidus temperature must be defined by the user.
The larger the difference between these two temperatures, the more stable the numerical behavior of the code but the more diffusive the solutions obtained.

!alert note
There is no need to define the solid fraction ($f_s$); it is defined internally in the solidification objects as $1 - f_l$.

!alert note
The capping of the liquid fraction is done via a smooth function in `MOOSE` to avoid issues with the Jacobian at discontinuities.

!syntax parameters /AuxKernels/NSLiquidFractionAux

!syntax inputs /AuxKernels/NSLiquidFractionAux

!syntax children /AuxKernels/NSLiquidFractionAux
