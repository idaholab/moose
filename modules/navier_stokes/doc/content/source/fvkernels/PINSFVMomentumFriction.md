# PINSFVMomentumFriction

This kernel adds the friction term to the porous media Navier Stokes momentum
equations. This kernel must be used with the canonical PINSFV variable set,
e.g. pressure and superficial velocity, and supports Darcy and
Forchheimer friction models in two flavors:

## Standard friction formulation

Set parameter: [!param](/FVKernels/PINSFVMomentumFriction/standard_friction_formulation) = 'true'

Darcy drag model
\begin{equation}
\label{darcy}
\epsilon F_i = - f_i \mu \epsilon \frac{v_{D,i}}{\epsilon} = -f_i \mu v_{D,i}
\end{equation}
Forchheimer drag model
\begin{equation}
\label{forchheimer}
\epsilon F_i = - f_i \frac{\rho}{2} \epsilon \frac{v_{D,i}}{\epsilon}\frac{|\vec{v_D}|}{\epsilon} = - f_i \frac{\rho}{2} v_{D,i} \frac{|\vec{v_D}|}{\epsilon}
\end{equation}

## Simplified friction formulation

Set parameter: [!param](/FVKernels/PINSFVMomentumFriction/standard_friction_formulation) = 'false'

Darcy drag model
\begin{equation}
\label{darcy2}
\epsilon F_i = - f_i \epsilon \frac{v_{D,i}}{\epsilon} = -f_i v_{D,i}
\end{equation}
Forchheimer drag model
\begin{equation}
\label{forchheimer2}
\epsilon F_i = - f_i \epsilon \frac{v_{D,i}}{\epsilon}\frac{|\vec{v_D}|}{\epsilon} = - f_i v_{D,i} \frac{|\vec{v_D}|}{\epsilon}
\end{equation}

where $F_i$ is the i-th component of the friction force (denoted by
$\mathbf{F_f}$ in [!eqref](pinsfv.md#eq:pinsfv_mom)), $f_i$ the friction factor,
which may be anisotropic, $\mu$ the fluid dynamic viscosity, $\rho$ the fluid density,
and $v_{D,i}$ the i-th component of the fluid superficial velocity.
We have used a negative sign to match the notation
used in [!eqref](pinsfv.md#eq:pinsfv_mom) where the friction force is on the
right-hand-side of the equation. When moved to the left-hand side, which is done
when setting up a Newton scheme, the term becomes positive which is what is
shown in the source code itself.  Darcy and Forchheimer terms represent
fundamentally different friction effects. Darcy is meant to represent viscous
effects and as shown in [darcy],[darcy2], it has a linear dependence on the fluid
velocity. Meanwhile, Forchheimer is meant to represent inertial effects and as
shown in [forchheimer], [forchheimer2] it has a quadratic dependence on velocity.

For the non-porous medium version of the above equations set parameter [!param](/FVKernels/PINSFVMomentumFriction/is_porous_medium)  to `false`.
(epsilon = 1)

## Computation of friction factors and pre-factors id=friction_example

To outline how friction factors for Darcy and Forchheimer may be calculated,
let's consider a specific example. We'll draw from the Ergun equation, which is
outlined [here](https://en.wikipedia.org/wiki/Ergun_equation). Let's consider
the form:

\begin{equation}
\Delta p = \frac{150\mu L}{d_p^2} \frac{(1-\epsilon)^2}{\epsilon^3} v_D + \frac{1.75 L \rho}{d_p} \frac{(1-\epsilon)}{\epsilon^3} |v_D| v_D
\end{equation}

where $L$ is the bed length, $\mu$ is the fluid dynamic viscosity and $d_p$ is
representative of the diameter of the pebbles in the pebble bed. We can divide
the equation through by $L$, recognize that $\Delta p$ denotes $p_0 - p_L$ such
that $\Delta p/L \approx -\nabla p$, multiply the equation through by
$-\epsilon$, move all terms to the left-hand-side, and do
some term manipulation in order to yield:

\begin{equation}
\epsilon \nabla p + 150\mu\epsilon\frac{(1-\epsilon)^2}{\epsilon^2 d_p^2} \frac{\vec{v_D}}{\epsilon} +
1.75\epsilon\frac{1-\epsilon}{\epsilon d_p} \rho \frac{|\vec{v}_D|}{\epsilon} \frac{\vec{v_D}}{\epsilon} = 0
\end{equation}

If we define the hydraulic diameter as $D_h = \frac{\epsilon d_p}{1 - \epsilon}$,
then the above equation can be rewritten as:

\begin{equation}
\epsilon \nabla p + \frac{150\mu}{D_h^2} \vec{v_D} +
\frac{1.75}{D_h} \rho \vec{v_D} \frac{|\vec{v}_D|}{\epsilon} = 0
\end{equation}

Let's introduce the interstitial fluid velocity $v = \vec{v_D} / \epsilon$ to rewrite
the above equation as:

\begin{equation}
\epsilon \nabla p + \epsilon\frac{150\mu}{D_h^2} \vec{v} +
\epsilon\frac{1.75}{D_h} \rho \vec{v} |\vec{v}| = 0
\end{equation}

Then dividing through by $\epsilon$:

\begin{equation}
\label{derived_ergun}
\nabla p + \frac{150\mu}{D_h^2} \vec{v} +
\frac{1.75}{D_h} \rho \vec{v} |\vec{v}| = 0
\end{equation}

We are now very close the forms for Darcy and Forchheimer espoused by
[Holzmann](https://holzmann-cfd.com/community/blog-and-tools/darcy-forchheimer)
and
[SimScale](https://www.simscale.com/knowledge-base/predict-darcy-and-forchheimer-coefficients-for-perforated-plates-using-analytical-approach/)
which is:

\begin{equation}
\label{holzmann}
\nabla p + \mu D \vec{v} + \frac{\rho}{2} F |\vec{v}| \vec{v}
\end{equation}

Looking at [holzmann] we can rearrange [derived_ergun]:

\begin{equation}
\nabla p + \mu \frac{150}{D_h^2} \vec{v} +
\frac{\rho}{2} \frac{2(1.75)}{D_h} \vec{v} |\vec{v}| = 0
\end{equation}

and arrive at the Ergun expression for the Darcy coefficient:

\begin{equation}
\frac{150}{D_h^2}
\end{equation}

and the Ergun expression for the Forchheimer coefficient:

\begin{equation}
\frac{2 \cdot 1.75}{D_h}
\end{equation}

where we have made the $2 \cdot 1.75$ multiplication explicit to make the 1.75 factor
from the [Ergun wikipedia page](https://en.wikipedia.org/wiki/Ergun_equation)
more recognizable. We perform a similar separation in the implementation of the
Ergun Forchheimer coefficient outlined in [FunctorErgunDragCoefficients.md].

!syntax parameters /FVKernels/PINSFVMomentumFriction

!syntax inputs /FVKernels/PINSFVMomentumFriction

!syntax children /FVKernels/PINSFVMomentumFriction
