# SCMFrictionPressureDrop

!syntax description /Postprocessors/SCMFrictionPressureDrop

The `SCMFrictionPressureDrop` postprocessor reports the cross-sectionally homogenized pressure
loss caused by axial wall friction and local form losses in a subchannel assembly. It excludes
axial acceleration, transient momentum storage, crossflow, and gravity terms.

!alert note
This postprocessor is intended primarily for coupling the subchannel assembly to THM or another
code that represents the assembly with a reduced momentum equation. The homogenized pressure
loss preserves the SCM assembly's total axial friction force when it is applied to the coupled
model's flow area.

For each subchannel $i$, it sums the friction contribution from each axial cell. For the implicit
momentum formulation, the cell contribution is reconstructed from the converged solution as

\begin{equation}
\Delta p_{\mathrm{loss},i,z}
=
\left(
f_{D,i,z}\frac{\Delta z}{D_{h,i,z}} + K_{i,z}
\right)
\frac{|\dot m_{i,z}^{\mathrm{out}}|}
     {2\rho_{i,z}S_{i,z}^2}
\left|
\alpha_{i,z}\dot m_{i,z}^{\mathrm{in}}
+ (1-\alpha_{i,z})\dot m_{i,z}^{\mathrm{out}}
\right|.
\end{equation}

Here, $\alpha$ is the momentum solver's spatial interpolation coefficient. The explicit
formulation instead uses
$\dot m^{\mathrm{out}}|\dot m^{\mathrm{out}}|/(2\rho S^2)$.
The friction force in an axial cell is $F_{\mathrm{loss},i,z} =
S_{i,z}\Delta p_{\mathrm{loss},i,z}$. The assembly pressure loss preserves the total axial
resistance force when reducing the subchannels to a single cross-sectionally averaged momentum
equation:

\begin{equation}
\Delta p_{\mathrm{loss}}
=
\sum_z
\frac{\sum_i |F_{\mathrm{loss},i,z}|}
     {\sum_i S_{i,z}}.
\end{equation}

This area homogenization is appropriate when the result is used as a friction source in a
one-dimensional momentum equation. Unlike mass-flow weighting, it does not apply an additional
weight to high-velocity subchannels and remains consistent when crossflow redistributes axial
mass flow.

The postprocessor uses the same friction closure, local loss coefficients, interpolation scheme,
and converged subchannel state as the momentum solver.

!syntax parameters /Postprocessors/SCMFrictionPressureDrop

!syntax inputs /Postprocessors/SCMFrictionPressureDrop

!syntax children /Postprocessors/SCMFrictionPressureDrop
