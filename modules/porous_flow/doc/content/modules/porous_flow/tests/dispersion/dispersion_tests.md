# Diffusion and hydrodynamic dispersion Tests

## Diffusion

The results of PorousFlow are compared with the classical diffusion profile
for a simple 1D model with mass diffusion. In this example, the left end of a 1D mesh is held at a constant mass fraction of 1, while the right hand end is
prescribed a zero mass fraction boundary condition. No advection takes place, so
mass transfer is by diffusion only. The input file:

!listing modules/porous_flow/test/tests/dispersion/diff01_action.i

This concentration profile has a well-known similarity solution given by
\begin{equation}
C(u) = \mathrm{erfc}\,(u),
\end{equation}
where $\mathrm{erfc}(u)$ is the complentary error function, and $u = x/(2 \sqrt{D t})$ is the
similarity solution, $x$ is distance, $t$ is time, and $D$ is the diffusion coefficient.

The comparison between PorousFlow and this analytical solution is presented in [fig:diff], where we observe a very good agreement between the two solutions.

!media dispersion/diffusion_fig.png style=width:80%;margin-left:10px caption=Mass fraction profile from diffusion only. id=fig:diff

## Hydrodynamic dispersion

The PorousFlow results are compared to known analytical solutions for simple problems in order to verify that the MOOSE implementation is working properly. For a simple 1D model with no diffusion and constant velocity $v$, an analytical solution for the mass fraction profile is given by [!citep](javendel)
\begin{equation}
\begin{array}{rcl}
C(x, t) & = & C_0 \left\{ \frac{1}{2} \mathrm{erfc}\,\left(\frac{x- v t}{2 \sqrt{D t}}\right) + \left(\frac{v^2 t}{\pi D}\right)^{1/2}
\exp \left(- \frac{\left[x - vt\right]^2}{4 D T}\right)\right.  \\
&&\ \  - \left. \frac{1}{2} \left(1 + \frac{v x}{D} + \frac{v^2 t}{D} \right) \exp\left(\frac{v x}{D}\right) \mathrm{erfc}\,\left(\frac{x+v t}{2 \sqrt{D T}}\right)\right\},
\end{array}
\end{equation}
where all parameters have been previously defined.

The input file:

!listing modules/porous_flow/test/tests/dispersion/disp01_heavy.i

The comparison between the PorousFlow and the analytical formula is presented in [fig:disp].  For the non-heavy case, the MOOSE results do not coincide with the analytical solution near the top and bottom of the concentration front due to numerical dispersion. If the number of elements in the mesh is increased and the time step size is reduced (the "heavy" case), numerical dispersion is reduced and a much closer fit to the analytical solution is obtained.

!media dispersion/dispersion_fig.png style=width:80%;margin-left:10px caption=Mass fraction profile from hydrodynamic dispersion only. id=fig:disp

!bibtex bibliography
