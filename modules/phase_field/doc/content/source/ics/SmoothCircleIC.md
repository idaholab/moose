# SmoothCircleIC

!syntax description /ICs/SmoothCircleIC

Insert a single disk or sphere with radius $R$ (`radius`) and a smooth interface
of width $w_i$ (`int_width`) with a user specified value range $[v_0, v_1]$ and
functional dependence of the interface (`profile`). $v_0$ (`outvalue`) is the
variable value outside of the disk/sphere and $v_1$ (`invalue`) is the value on
the interior.

### Cosine profile (`COS`)

\begin{equation}
v(\vec r) = x\begin{cases}
        v_1, & \text{for } |\vec r| \leq R - \frac w2\\
        v_0 +  \frac12(v_1 - v_0) \left(1.0 + \cos(\pi\frac{|\vec r| - R\ + \frac w2}w)\right)
, & \text{for } R - \frac w2\lt |\vec r| \lt R + \frac w2\\
        v_0, & \text{for } R + \frac w2 \leq |\vec r|
        \end{cases}
\end{equation}

### Hyperbolic tangent profile (`TANH`)

\begin{equation}
\label{eq-tanh}
v(\vec r) = (v_1 - v_0) \frac12 \left(\tanh(\pi \frac{R - |\vec r|}w ) + 1\right) + v_0;
\end{equation}

In both cases $v(\vec r)$ is the initial condition value at $\vec r + \vec r_0$,
where $\vec r_0$ is the center point (`x1`, `y1`, `z1`) of the circle (making
$\vec r$ a radius vector).

!media /tanh_interface.png style=width:50%;margin-left:20px;
       caption=The hyperbolic tangent interface is scaled to have the same slope at
       its midpoint as the cosine interface.

For the `COS` interface, which is strictly zero outside the particle and
strictly inside the particle the interface width `int_width` denotes exactly the
width of the transition region where the order parameter values are in the open
interval $(v_0,v_1)$. The `TANH` interface function is asymptotic and requires a
different definition for the interface width. Here we chose to have the slope at
the midpoint match up with the midpoint slope of the `COS` interface (giving
rise to the factor of $\pi$ in eq. [eq-tanh]).

!syntax parameters /ICs/SmoothCircleIC

!syntax inputs /ICs/SmoothCircleIC

!syntax children /ICs/SmoothCircleIC

!bibtex bibliography
