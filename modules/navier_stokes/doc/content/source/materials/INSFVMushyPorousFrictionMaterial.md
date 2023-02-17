# INSFVMushyPorousFrictionMaterial

!syntax description /Materials/INSFVMushyPorousFrictionMaterial

This material computes the `Darcy_coefficient` and the `Forchheimer_coefficient` for a solidification problem.
The model uses a mushy-zone approach to compute the friction coefficients.

The coefficients are defined as follows:

\begin{equation}
D = \frac{\mu}{K} \,,
\end{equation}

where the permeability $K$ is defined as follows:

\begin{equation}
K = \frac{f_l^3}{f_s^2 F_k c_s} \,,
\end{equation}

where $f_l$ and $f_s$ are the liquid and solid fraction,
the switching function is defined as $F_k = 0.5 * \operatorname{atan}(s (f_s - f_{s,crit}) / \pi)$ with $s=100$ and $f_{s,crit}=0.27$
and $c_s = c / (\lambda)^2$ with $c = 180$ and the dendrite spacing is $\lambda$.
By default, we set $\lambda = 10^{-4}$. However, this coefficient may be tuned by the user if experimental data is available.

The Forchheimer coefficient is defined as follows:

\begin{equation}
F = C_F \frac{\rho_l}{\sqrt{K}} \,,
\end{equation}

where the Ergun coefficient is $C_F = 0.55$ and $\rho_l$ is the density of the liquid phase.

!alert note
This material is compatible with [INSFVMomentumFriction.md] which multiplies the
incoming Forchheimer coefficient by the velocity magnitude; it is *incompatible*
with [PINSFVMomentumFriction.md] which assumes the incoming Forchheimer
coefficient already includes multiplication by the velocity magnitude.

!syntax parameters /Materials/INSFVMushyPorousFrictionMaterial

!syntax inputs /Materials/INSFVMushyPorousFrictionMaterial

!syntax children /Materials/INSFVMushyPorousFrictionMaterial
