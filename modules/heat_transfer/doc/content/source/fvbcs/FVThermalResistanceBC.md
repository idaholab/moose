# FVThermalResistanceBC

!syntax description /FVBCs/FVThermalResistanceBC

# Description

Heat flux boundary condition for the fluid or solid energy conservation equations. When used
with the fluid energy equation, this boundary condition specifies

\begin{equation}
-\int_\Gamma\kappa_f \nabla T_f\cdot\hat{n}d\Gamma\ ,
\end{equation}

where

\begin{equation}
-\kappa_f\nabla T_f\cdot\hat{n}=\tilde{q}\ ,
\end{equation}

where $\tilde{q}$ is the imposed heat flux, to be discussed shortly.
When used with the solid energy equation, this boundary condition specifies

\begin{equation}
-\int_\Gamma\kappa_s\cdot\nabla T_s\cdot\hat{n}d\Gamma\ ,
\end{equation}

where

\begin{equation}
-\kappa_s\cdot\nabla T_s\cdot\hat{n}=\tilde{q}\ .
\end{equation}

The heat flux $\tilde{q}$ is computed based on the thermal resistance concept, where heat
is assumed to conduct through multiple parallel slabs of constant-property solid in series,
followed by parallel convection and radiation to an ambient temperature. For $N$ conducting
layers, the conduction thermal resistance $R_c$ is computed for Cartesian slabs as

\begin{equation}
R_c=\sum_{i=1}^N\frac{\Delta x_i}{k_i}\ ,
\end{equation}

where $\Delta x_i$ is the thickness of each layer and $k_i$ is the thermal conductivity of
each layer. For cylindrical annuli, the conduction thermal resistance is computed as

\begin{equation}
R_c=\sum_{i=1}^N\frac{\ln{\left(\frac{\Delta x_i+r_i}{r_i}\right)}}{2 \pi k_i}\ ,
\end{equation}

where $r_i$ is the inner radius corresponding to each layer. The inner radius of the very
first cylindrical annulus is specified by the `inner_radius` parameter.

!alert warning
The inner radius of a cylindrical bed is also provided by the `inner_radius` parameter for
setting porosity functions and other near-wall behavior. To prevent errors associated with
mixing the two different interpretations of the `inner_radius` parameter, it's best to avoid
setting this parameter in the `GlobalParams` input file block.

The parallel convection and radiation resistance from the surface of the conduction
layers, or $R_p$, is then computed as

\begin{equation}
R_p=\frac{1}{R\left(h_r+h_c\right)}\ ,
\end{equation}

where $R$ is equal to the cylindrical annuli outer radius for cylindrical geometries and
unity for Cartesian geometries; $h_c$ is the surface convection coefficient; and $h_r$ is
the radiation heat transfer coefficient,

\begin{equation}
h_r=\varepsilon\sigma\left(T_s^2+T_\infty^2\right)\left(T_s+T_\infty\right)\ ,
\end{equation}

where $\varepsilon$ is the emissivity of the last conduction layer, $\sigma$ is the
Stefan-Boltzmann constant, $T_s$ is the surface temperature of the last conduction layer,
and $T_\infty$ is the far-field temperature.

The surface temperature is implicitly dependent on the heat flux, so an underrelaxed
fixed point iteration is used to solve for $T_s$ at each quadrature point.

!alert warning
The thermal resistance concept does not apply to conducting slabs with heat sources
or systems undergoing transients. This boundary condition should only be applied to steady-state
or pseudo-steady transients.

!alert note
The `emissivity` parameter represents the emissivity of the _surface_, or last layer,
of the conducting slabs.

## Example input syntax

In this example, we set thermal resistance boundary conditions on the `top` and `left` boundaries.
We specify emissivity, thermal conductivity and convective heat transfer coefficients to model those
three heat transfer mechanisms where present (they are set to 0 when absent).

The inner iteration parameters are exposed for the `left` boundary. This is only meant to be used
to fine-tune a simulation for performance, or if convergence difficulties are encountered for that
boundary.

!listing test/tests/fvbcs/fv_thermal_resistance/test.i block=FVBCs

!syntax parameters /FVBCs/FVThermalResistanceBC

!syntax inputs /FVBCs/FVThermalResistanceBC

!syntax children /FVBCs/FVThermalResistanceBC
