# Thermal Expansion Problem

## Summary

Solves a coupled 3D thermomechanical problem for the bending of a steel cantilever beam whose top
face is heated. The temperature and the displacement are solved for together in a single linear
system, with the thermal strain coupled into the momentum balance by a transposed
[MFEMVectorDivergenceKernel.md] and an [MFEMVectorDomainLFGradKernel.md] for the stress-free
reference temperature.

## Description

The beam occupies $\Omega = [0, L] \times [0, b] \times [0, h]$ with $L = 8$ m and $b = h = 1$ m, and
is free of stress at the reference temperature $T_\mathrm{ref} = 293.15$ K. Its temperature $T$
solves the steady heat equation

\begin{equation}
\begin{split}
-\vec \nabla \cdot (k \vec \nabla T) = 0 \,\,\, &\mathrm{on}\,\, \Omega \\
T = T_\mathrm{ref} + \Delta T_\mathrm{top} \,\,\, &\mathrm{on}\,\, \Gamma_\mathrm{top} \\
T = T_\mathrm{ref} \,\,\, &\mathrm{on}\,\, \Gamma_\mathrm{bottom}
\end{split}
\end{equation}

with zero heat flux on the remaining faces, giving a temperature rise $\Delta T = T - T_\mathrm{ref}$
that varies linearly through the thickness, $\Delta T = \Delta T_\mathrm{top} z / h$.

The displacement $\vec u$ solves the small-deformation equilibrium equations

\begin{equation}
\begin{split}
-\partial_j \sigma_{ij} = 0 \,\,\, &\mathrm{on}\,\, \Omega \\
u_i = 0 \,\,\, &\mathrm{on}\,\, \Gamma_\mathrm{left} \\
\sigma_{ij} \hat n_j = 0 \,\,\, &\mathrm{on}\,\, \partial\Omega \setminus \Gamma_\mathrm{left}
\end{split}
\end{equation}

where the beam is clamped at $x = 0$ and the isotropic stress is reduced by the thermal strain
$\alpha \Delta T \delta_{ij}$,

\begin{equation}
\sigma_{ij} = \lambda \delta_{ij} \varepsilon_{kk} + 2 \mu \varepsilon_{ij}
- \beta (T - T_\mathrm{ref}) \delta_{ij},
\,\,\,\, \beta = (3 \lambda + 2 \mu) \alpha = \frac{E \alpha}{1 - 2\nu}
\end{equation}

Here $\alpha$ is the linear thermal expansion coefficient and $\beta$ is the stress a unit
temperature rise induces in a fully constrained body.

The coupled weak form is

\begin{equation}
\begin{split}
(k \vec \nabla T, \vec \nabla w)_\Omega &= 0 \,\,\, \forall w \in W \\
(C_{ijkl} \varepsilon_{kl}, \partial_j v_i)_\Omega - (\beta T, \vec \nabla \cdot \vec v)_\Omega
&= -(\beta T_\mathrm{ref}, \vec \nabla \cdot \vec v)_\Omega \,\,\, \forall \vec v \in V
\end{split}
\end{equation}

The first term on the left of the second equation is added by [MFEMLinearElasticityKernel.md]. The
second is added by [MFEMVectorDivergenceKernel.md] with `coefficient` $= -\beta$, `trial_variable`
set to the temperature, and `transpose = true`, so that the test and trial spaces of the
$(k \vec \nabla \cdot \vec u, v)_\Omega$ form are swapped. The right hand side is added by
[MFEMVectorDomainLFGradKernel.md] with `coefficient` $= -\beta T_\mathrm{ref}$. The temperature does
not depend on the displacement, so the coupled system is block lower triangular; it is solved
directly with [MFEMMUMPS.md].

The temperature is discretized with first-order $H^1$ elements and the displacement with
second-order $H^1$ elements, so that the thermal strain lies in the same polynomial space as the
displacement gradient.

## Results

A beam free to deform under a temperature change that is linear in $z$ bends into a circular arc
without stress. Away from the clamped root, Euler-Bernoulli beam theory therefore gives the
centreline displacement exactly up to a rigid-body motion:

\begin{equation}
u_x = \frac{\alpha \Delta T_\mathrm{top}}{2} x, \,\,\,\,
u_z = -\frac{\alpha \Delta T_\mathrm{top}}{2 h} x^2
\end{equation}

With $\alpha = 1.2 \times 10^{-5}$ K$^{-1}$ and $\Delta T_\mathrm{top} = 100$ K, the computed
centreline axial strain ($6.0 \times 10^{-4}$) and curvature ($1.2 \times 10^{-3}$ m$^{-1}$) match
these expressions. The hotter top face expands more than the bottom face, so the tip deflects
downwards, by $3.92 \times 10^{-2}$ m. This is about 2% more than the beam-theory value of
$3.84 \times 10^{-2}$ m: the clamp prevents the root cross-section from expanding and warping, which
stresses the beam locally and adds a small rotation at the root.

## Example File

!listing test/tests/mfem/kernels/thermal_expansion.i
