# Grain Boundary Anisotropy

The physical fidelity and accuracy of the predicted grain growth depend on the
accuracy of phase field parameters. In the Ginzburg-Landau equation

\begin{equation}
\frac{\partial \eta_i}{\partial t}=-L_i\left[\mu\left(\eta_i^3-\eta_i+2\gamma_i\eta_i\sum_{j\neq i}{\eta_j^2}\right)-\nabla\cdot\kappa_i\nabla\eta_i\right],
\end{equation}

The parameters $L_i$, $\mu$, $\gamma_i$ and $\kappa_i$ should be determined from
GB properties (energy $\sigma_{gb}$ and mobility $m_{gb}$), which are usually
obtained from Molecular Dynamics computation. There are different methods around
to relate these parameters with GB properties. The only requirement for the
parametrization is that the parameterized model can reproduce the desired GB
properties.

The phase field module implements the parameter determination method developed
by Moelans et al. [!cite](moelans_quantitative_2008). For isotropic GB
properties, these phase field parameters are expressed as

\begin{equation}
  \gamma  = 1.5,
\end{equation}

\begin{equation}
  \kappa \approx \frac{3}{4} \sigma_{gb} l_{gb},
\end{equation}

\begin{equation}
  L \approx \frac{4}{3} \frac{m_{gb}}{l_{gb}},
\end{equation}

\begin{equation}
  \mu \approx \frac{3}{4} \frac{1}{f_{0,saddle}(\gamma)} \frac{\sigma_{gb}}{l_{gb}},
\end{equation}

where $l_{gb}$ is the diffuse GB width, and $f_{0,saddle}(\gamma)$ is the
chemical free energy at the saddle point. The parameters $(L,\kappa,\gamma,\mu)$
are uniform across the system. The isotropic material model is implemented in
[`GBEvolution`](GBEvolution.md).

!media media/phase_field/double_circle_grain.png
       caption=Double-circle grain test
       style=width:30%;padding-left:20px;float:right

For systems with misorientation dependence of GB properties, a set of GB
energies $\sigma_{gb,k}$ and mobilities $m_{gb,k}$ as functions of
misorientation angle $k$ should be considered. An iterative procedure was used
to calculate the parameters $(L_k,\kappa_k,\gamma_k,\mu)$ for each
misorientation $k$, which are then interpolated through the following functions
to obtain the continuous phase field parameters at each integration point,

\begin{equation}
	\psi=\frac{\sum\limits_{i=1}^{p}{\sum\limits_{j>i}^{p}{\psi_k\eta_i^2\eta_j^2}}}{\sum\limits_{i=1}^{p}{\sum\limits_{j>i}^{p}{\eta_i^2\eta_j^2}}},
\end{equation}

where $\psi_k$ represents parameters $L_k$, $\kappa_k$, or $\gamma_k$. $\mu$ is
set to be constant. More details on the parametrization are referred to Moelans
2008.

A double-circle grain test is used to validate the implementation of GB
anisotropy with misorientation. In the right figure, two circular grains of the
same initial size are embedded in a matrix grain, and they would shrink under
the driving force of reducing total GB area. It can be proven that the changing
grain area $A$ of either circular grain should follow

\begin{equation}
	A=A_0-2\pi m_{gb}\sigma_{gb}\cdot t,
\end{equation}

!media media/phase_field/area_evolution_grain1.png
       caption=Area evolution of grains with different mobilities
       style=width:40%;padding-left:20px;float:right

!media media/phase_field/area_evolution_grain2.png
       caption=Area evolution of grain with varying grain boundary energies
       style=width:40%;padding-left:20px;clear:right;float:right

where $A_0$ is the initial grain area. We performed two tests where different
properties were assigned to GBs formed by the two circular grains and the
matrix. First, these two GBs have the same boundary energy but different
mobilities ($m_2 = 2\cdot m_1$). Second, they obtain the same GB mobility but
different energies ($\sigma_2 = 2 \cdot \sigma_1$). The simulated and analytical
grain area evolution are compared in following figures.

Apart from little non-linearity at the beginning of the 2$^{nd}$ test simulation
due to GB relaxation from symmetrical to non-symmetrical profile, the grain area
evolutions agree quite well with analytical solution, which shows the phase
field parameters determined from this method reproduces exactly the desired GB
properties.

The anisotropic GB properties are implemented in material
[`GBAnisotropy`](GBAnisotropy.md) to determine the phase field parameters.
