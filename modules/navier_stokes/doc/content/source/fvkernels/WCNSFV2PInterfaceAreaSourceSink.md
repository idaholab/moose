# WCNSFV2PInterfaceAreaSourceSink

The interfacial area concentration is defined as the interface area between
two phases per unit volume, i.e.,$[\xi_p] = \frac{m^2}{m^3}$. This parameter is important
for predicting mass, momentum, and energy transfer at interfaces in two-phase flows.

The general equation for interfacial area concentration transport via the mixture model reads as follows:

\begin{equation}
\frac{\partial (\rho_d \xi_p)}{\partial t} + \nabla \cdot \left( \rho_d \vec{u} \xi_p \right) - \nabla \left( D_b \nabla \xi_p \right) =
-\frac{1}{3} \frac{D \rho_d}{Dt} + S_T + \rho_d (S_C + S_B)\,,
\end{equation}

where:

- $\rho_d$ is the density of the dispersed phase $d$, e.g., the density of the gas in bubbles,
- $t$ is time,
- $\vec{u}$ is the velocity vector,
- $D_b$ is a diffusion coefficient for the interface area concentration, which may be assumed to be `0` if unknown,
- $\frac{D (\bullet)}{Dt} = \frac{\partial (\bullet)}{\partial t} + \vec{u} \cdot \nabla (\bullet)$ is the material derivative,
- $S_T$, $S_C$, and $S_B$ are the interface area concentration sources due to mass transfer, coalescence, and breakage, respectively.

The terms on the left-hand side of this equation are modeled via standard kernels for the mixture model.
For example, in an open flow case, the time derivative may be modeled using [FVFunctorTimeKernel.md],
the advection term using [INSFVScalarFieldAdvection.md], and the diffusion term using [FVDiffusion.md].

The terms on the right-hand side are modeled using a multidimensional version of
Hibiki and Ishii's model [!citep](hibiki2000interface).
In this one, the sources are approximated as follows:


## Interface area concentration source due to mass transfer

The interface area concentration source due to mass transfer is modeled as follows:

\begin{equation}
S_T =
\begin{cases}
  \frac{6 \alpha_d}{d_p}, & \text{if } \alpha_d < \alpha_d^{co} \\
  \frac{2}{3} \cdot h^{b \rightarrow d} \left( \frac{1}{\alpha_d} - 2.0 \right), & \text{otherwise}
\end{cases}\,,
\end{equation}

where:

- $\alpha_d$ is the volumetric fraction of the dispersed phase, e.g., the void fraction if the dispersed phase is a gas,
- $\alpha_d^{co}$ is a cutoff fraction for mass transfer model selection,
- $d_p$ is the best estimate for the dispersed phase particle diameter,
- $h^{b \rightarrow d}$ is the mass exchange coefficient from the bulk to the dispersed phase,

The cutoff volumetric fraction $\alpha_d^{co}$ is included because the mass transfer term
in Hibiki and Ishii's model is not physical for low volumetric fractions of the dispersed phase.
Below the cutoff limit, the dispersed phase is modeled as spherical particles.

!alert note
The user should select the cutoff volumetric fraction $\alpha_d^{co}$ as the limit at which
the modeled flows transition away from bubbly flow, i.e., below the cutoff limit there is an
implicit assumption that the flow behaves as a bubbly flow.


## Interface area concentration source due to coalescence

The reduction in interface area concentration due to coalescence of the dispersed phase is modeled as follows:

\begin{equation}
S_C = -\left( \frac{\alpha_d}{\xi_p} \right)^2 \frac{\Gamma_C \alpha_d^2 u_{\epsilon}}{\tilde{d_p}^{11/3} (\alpha_d^{max}- \alpha_d)}
\operatorname{exp} \left( -K_C \frac{\tilde{d_p}^{5/3} \rho_b^{1/2} u_{\epsilon}^{1/3}}{\sigma^{1/2}} \right)\,,
\end{equation}

where:

- $\alpha_d$ is the volumetric fraction of the dispersed phase, e.g., the void fraction if the dispersed phase is a gas,
- $u_{\epsilon} = \left( \| \vec{u} \| \ell \| \nabla p \| / \rho_m \right)^{1/3}$ is the friction velocity due to pressure gradients, with $\| \vec{u} \|$ being the norm of the velocity vector, $\ell$ a characteristic mixing length, $\| \nabla p \|$ the norm of the pressure gradient, and $\rho_m$ the mixture density,
- $\tilde{d_p} = \psi \frac{\alpha_d}{\xi_p}$ is the model estimate for the dispersed phase particle diameter, with $\psi$ being a shape factor, which is, for example, $\psi=6$ for spherical particles,
- $\alpha_d^{max}$ is the maximum volumetric fraction admitted by the model, in absence of data we recommend taking $\alpha_d^{max}=1$,
- $\rho_b$ is the bulk phase density, e.g., for air bubbles in a water flow it would be the density of water,
- $\sigma$ is the surface tension between the two phases,
- $\Gamma_C = 0.188$ and $K_C = 0.129$ are closure coefficients of the model.

!alert note
Many of the parameters in the coalescence model are provided as functors,
which means that spatially dependent fields may be specified for these parameters.
However, the validation of this model has only been performed using constant parameters.

## Interface area concentration source due to breakage

The increase in interface area concentration due to breakage of the dispersed phase is modeled as follows:

\begin{equation}
S_B = \left( \frac{\alpha_d}{\xi_p} \right)^2 \frac{\Gamma_B \alpha_d (1 - \alpha_d) u_{\epsilon}}{\tilde{d_p}^{11/3} (\alpha_d^{max}- \alpha_d)}
\operatorname{exp} \left( -K_B \frac{\sigma}{\rho_b \tilde{d_p}^{5/3} u_{\epsilon}^{2/3}} \right)\,,
\end{equation}

where:

- $\alpha_d$ is the volumetric fraction of the dispersed phase, e.g., the void fraction if the dispersed phase is a gas,
- $u_{\epsilon} = \left( \| \vec{u} \| \ell \| \nabla p \| / \rho_m \right)^{1/3}$ is the friction velocity due to pressure gradients, with $\| \vec{u} \|$ being the norm of the velocity vector, $\ell$ a characteristic mixing length, $\| \nabla p \|$ the norm of the pressure gradient, and $\rho_m$ the mixture density,
- $\tilde{d_p} = \psi \frac{\alpha_d}{\xi_p}$ is the model estimate for the dispersed phase particle diameter, with $\psi$ being a shape factor, which is, for example, $\psi=6$ for spherical particles,
- $\alpha_d^{max}$ is the maximum volumetric fraction admitted by the model, in absence of data we recommend taking $\alpha_d^{max}=1$,
- $\rho_b$ is the bulk phase density, e.g., for air bubbles in a water flow it would be the density of water,
- $\sigma$ is the surface tension between the two phases,
- $\Gamma_B = 0.264$ and $K_B = 1.370$ are closure coefficients of the model.

!alert note
Many of the parameters in the breakage model are provided as functors,
which means that spatially dependent fields may be specified for these parameters.
However, the validation of this model has only been performed using constant parameters.


!syntax parameters /FVKernels/WCNSFV2PInterfaceAreaSourceSink

!syntax inputs /FVKernels/WCNSFV2PInterfaceAreaSourceSink

!syntax children /FVKernels/WCNSFV2PInterfaceAreaSourceSink
