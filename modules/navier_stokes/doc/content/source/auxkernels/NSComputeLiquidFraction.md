# NSComputeLiquidFraction

!syntax description /AuxKernels/NSComputeLiquidFraction

The liquid fraction is defined as follows:

\begin{equation}
f_l =
\begin{cases}
f_l = 0, \text{ if } T < T_{solidus}, \\
f_l = \frac{T - T_{solidus}}{T_{liquidus} - T_{solidus},}, \text{ if } T \in (T_{solidus}, T_{liquidus}), \\
f_l = 1, \text{ if } T > T_{liquidus}, \\
\end{cases}
\end{equation}

where $T_{liquidus}$ and $T_{solidus} < T_{liquidus}$ are the liquidus and solidus temperatures.

The liquidus and solidus temperature must be defined by the user.
The larger the difference between these two temperatures, the more stable the numerical behavior of the code but the more diffusive the solutions obtained.

*Note 1*: there is no need to define the solid fraction ($f_s$), this one gets defined internally in the solidification objects as $1 - f_l$.

*Note 2*: the capping of the liquid fraction is done via a smooth function in `MOOSE` to avoid issues with the Jacobian at discontinuities.

!syntax parameters /AuxKernels/NSComputeLiquidFraction

!syntax inputs /AuxKernels/NSComputeLiquidFraction

!syntax children /AuxKernels/NSComputeLiquidFraction
