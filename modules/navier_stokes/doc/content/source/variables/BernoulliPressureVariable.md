# BernoulliPressureVariable

This variable type is specific to the porous media incompressible Navier Stokes
equations. When used instead of a typical finite volume variable, faces for
which the neighboring elements have different porosity values will be treated as
either Dirichlet or extrapolated boundary faces. When this variable is queried for a face value
on the downwind side of the face, only downwind information is used to
extrapolate and reconstruct the downwind side face value. The upwind side face
value is computed using the reconstructed downwind face pressure value and the
Bernoulli equation:

\begin{equation}
p_1 + \frac{1}{2}\rho_1\vec{v}_1^2 = p_2 + \frac{1}{2}\rho_2\vec{v}_2^2
\end{equation}

where $p$ is the pressure, $\rho$ is the density, and $\vec{v}$ is the
interstitial velocity (not the superficial velocity). Bernoulli's equation
typically contains gravitational terms; however, we have omitted them under the
assumption that $\rho_1 = \rho_2$ which should be true when density does not
depend on pressure (the incompressible or "weakly" compressible case).

!syntax parameters /Variables/BernoulliPressureVariable

!syntax inputs /Variables/BernoulliPressureVariable

!syntax children /Variables/BernoulliPressureVariable
