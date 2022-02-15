# PINSFVMomentumFrictionCorrection

!syntax description /FVKernels/PINSFVMomentumFrictionCorrection

Since the friction corrector in the pressure interpolation scheme is not explained elsewhere,
we provide a complete explanation in this document.

### Pressure interpolation at the interfaces

Assume $\vec{d}_e$ to be the vector position of the center node of the current cell,
$\vec{d}_n$ to be the center node of the neighbor cell,
$\vec{d}_{ne} = \vec{d}_n - \vec{d}_e$ to be the distance vector between both cells,
and $\vec{d}_{n,int} = \vec{d}_n - \vec{d}_{ne}$ and $\vec{d}_{e,int} = \vec{d}_e - \vec{d}_{ne}$
the distance vectors to the interface.
Assuming that the pressure for the collocated node at the current and neighbor cells are
$p_e$ and $p_n$, respectively, the pressure at the interface can be computed as follows:

\begin{equation}
p_ne = f(\vec{d}_{n,int}) p_n + g(\vec{d}_{e,int}) p_e \,,
\end{equation}

where $f$ and $g$ are some generic functions.
In MOOSE finite volume, and in cell-centered finite volume methods generally, cell-centered fields are often interpolated to faces
using arithmetic means, i.e.,
$f(\vec{d}_{n,int}) = \frac{|\vec{d}_{e,int}|}{|\vec{d}_{ne}|}$
and $f(\vec{d}_{e,int}) = \frac{|\vec{d}_{n,int}|}{|\vec{d}_{ne}|} = 1 - \frac{|\vec{d}_{e,int}|}{|\vec{d}_{ne}|}$,
which yields:

\begin{equation}
\tilde{p}_{ne} = \frac{|\vec{d}_{e,int}|}{|\vec{d}_{ne}|} p_n + \frac{|\vec{d}_{n,int}|}{|\vec{d}_{ne}|} p_e \,.
\end{equation}

This interpolation is exact as long as there is no variation in the pressure gradient
between the current and neighbor cells.
Otherwise, it introduces an interpolation error proportional to the difference in the gradients
for the current and neighbor cells and the distance between cells.
Large differences in pressure gradient can happen for instance in sharp porous media, due to differences in friction loss and/or effective flow area.

### Correction of the interpolated pressure at the interfaces

The pressure correction interpolation is based on the introduction of a correction term
provided by considering the upwind and downwind linear interpolation at the cell faces.


Upwinding the pressure from the current cell to the interface yields:

\begin{equation}
p_{ne}^u = p_e + \alpha_c \nabla{p}_e \vec{d}_{e,int} \,,
\end{equation}

where $\alpha_c \in \mathbb{R}^+$ is a factor that defines the advection character in the pressure interpolation.

Similarly, approximating the pressure at the interface by down-winding from the neighbor yields:

\begin{equation}
p_{ne}^d = p_n - \alpha_c \nabla{p}_n \vec{d}_{n,int} \,.
\end{equation}

Now we define the interface pressure as the arithmetic means of the upwind and downwind pressures
to yield:

\begin{equation}
p_{ne} =
\frac{|\vec{d}_{e,int}|}{|\vec{d}_{ne}|} p_{ne}^d + \frac{|\vec{d}_{n,int}|}{|\vec{d}_{ne}|} p_{ne}^u
= \frac{|\vec{d}_{e,int}|}{|\vec{d}_{ne}|} p_n + \frac{|\vec{d}_{n,int}|}{|\vec{d}_{ne}|} p_e
- \alpha_c \frac{|\vec{d}_{e,int}|}{|\vec{d}_{ne}|} \nabla{p}_n \vec{d}_{n,int}
+ \alpha_c \frac{|\vec{d}_{n,int}|}{|\vec{d}_{ne}|} \nabla{p}_e \vec{d}_{e,int}
= \tilde{p}_{ne}
- \alpha_c \frac{|\vec{d}_{e,int}|}{|\vec{d}_{ne}|} \nabla{p}_n \vec{d}_{n,int}
+ \alpha_c \frac{|\vec{d}_{n,int}|}{|\vec{d}_{ne}|} \nabla{p}_e \vec{d}_{e,int}
\end{equation}

Next, we approximate the pressure gradients by the body forces as
$\nabla{p}_e \approx \vec{F}_e$ and $\nabla{p}_n \approx \vec{F}_n$,
i.e., we neglect the effects of flow acceleration and inertia on the pressure correction.
This yields the following interpolation field for the pressure at the interfaces:

\begin{equation}
p_{ne} =
\tilde{p}_{ne}
+ \frac{\alpha_c}{|\vec{d}_{ne}|}
(\vec{F}_e \vec{d}_{e,int} |\vec{d}_{n,int}| - \vec{F}_n \vec{d}_{n,int} |\vec{d}_{e,int}|)
\end{equation}

### Setting the parameter $\alpha_c$

During finite-volume integration, we will sum over the control volume the fluxes at the faces.
For the correction in the pressure interpolation this yields:

\begin{equation}
\sum_{int} \frac{\alpha_c}{|\vec{d}_{ne}|}
(\vec{F}_e \vec{d}_{e,int} |\vec{d}_{n,int}| - \vec{F}_n \vec{d}_{n,int} |\vec{d}_{e,int}|) \,,
\end{equation}

which defines a diffusion term of the form:

\begin{equation}
\nabla \cdot (\alpha_c \vec{d}_{e,int} |\vec{d}_{n,int}|) \nabla \vec{F} \,.
\end{equation}

When increasing the value of the advection parameter $\alpha_c$,
oscillations are reduced as the arithmetic means interpolation error becomes less significant
with respect to the advection interpolation.
However, note that in a sharp porous media, the term $\vec{F}$ may be sharp and discontinuous.
Hence, increasing the value of the advection parameter adds stiffness to the problem,
which deteriorates the performance of iterative solvers.
In practice, a value of $alpha_c \approx 10$ have shown good performance in reducing
porous-media-driven oscillations without causing convergence issues

!syntax parameters /FVKernels/PINSFVMomentumFrictionCorrection

!syntax inputs /FVKernels/PINSFVMomentumFrictionCorrection

!syntax children /FVKernels/PINSFVMomentumFrictionCorrection
