# Compute Cracked Stress

!syntax description /Materials/ComputeCrackedStress

## Description

This material implements a phase field fracture model that can include anisotropic elasticity tensors, modifying the stress and computing the free energy derivatives required for the model. It works with the standard phase field
kernels for nonconserved variables. In the model, a nonconserved order parameter $c$ defines the
crack, where $c = 0$ in undamaged material and $c = 1$ in cracked material. Cracked material can
sustain a compressive stress, but not a tensile one. $c$ evolves to minimize the elastic free energy
of the system.

This model takes the stress and Jacobian_mult that were calculated by another material and modifies them to include cracks.

## Model Summary

In the model, the uncracked stress $\sigma_0$ is provided by another material. It is decomposed into its compressive $(-)$ and tensile $(+)$ parts using a spectral decomposition
\begin{equation}
\boldsymbol{\sigma}_0 = \boldsymbol{Q} \boldsymbol{\Lambda} \boldsymbol{Q}^T,
\end{equation}
The compressive and tensile parts of the stress are computed from postive and negative projection tensors (computed from the spectral decomposition) according to
\begin{equation}
	\boldsymbol{\sigma}^+ = \mathbf{P}^+ \boldsymbol{\sigma}_0
\end{equation}
\begin{equation}
	\boldsymbol{\sigma}^- = \mathbf{P}^- \boldsymbol{\sigma}_0,
\end{equation}

## Free Energy Calculation

The total strain energy density is defined as
\begin{equation}
\Psi = [(1-c)^2(1-k) + k] \Psi^{+} +\Psi^{-},
\end{equation}
where $\psi^{+}$ is the strain energy due to tensile stress, $\psi^{-}$ is the strain energy due to
compressive stress, and $k \ll 0$ is a parameter used to avoid non-positive definiteness at or near
complete damage. The compressive and tensile strain energies are determined from:
\begin{equation}
\psi^{+} = \frac{1}{2} \boldsymbol{\sigma}^{+} : \boldsymbol{\epsilon}
\end{equation}
\begin{equation}
\psi^{-} = \frac{1}{2} \boldsymbol{\sigma}^{-} : \boldsymbol{\epsilon}.
\end{equation}

The crack energy density is defined as
\begin{equation}
\gamma = g_c \frac{1}{2l}c^2 + g_c \frac{l}{2} {| \nabla c |}^2,
\end{equation}
where $l$ is the width of the crack interface and $g_c$ is a parameter related to the energy release rate.

The total local free energy density is defined as
\begin{equation}
\begin{aligned}
F =& \Psi + \gamma \\
  =&[(1-c)^2(1-k) + k] \Psi^{+} +\Psi^{-} + \frac{g_c}{2l}c^2 + \frac{g_c l}{2} {|{\nabla c}|}^2.
\end{aligned}
\end{equation}

## Stress Definition

To be thermodynamically consistent, the stress is related to the deformation energy density according
to
\begin{equation}
  \boldsymbol{\sigma} = \frac{\partial \psi_e}{\partial \boldsymbol{\epsilon}} = ((1-c)^2(1-k) + k)\frac{\partial \psi^+}{\partial \boldsymbol{\epsilon}} + \frac{\partial \psi^-}{\partial \boldsymbol{\epsilon}}.
\end{equation}
Since
\begin{equation}
	\frac{\partial \psi^+}{\partial \boldsymbol{\epsilon}} = \boldsymbol{\sigma}^+
\end{equation}
\begin{equation}
	\frac{\partial \psi^-}{\partial \boldsymbol{\epsilon}} = \boldsymbol{\sigma}^-,
\end{equation}
then,
\begin{equation}
	\boldsymbol{\sigma} = ((1-c)^2(1-k) + k)\boldsymbol{\sigma}^+ + \boldsymbol{\sigma}^-
\end{equation}

The Jacobian matrix for the stress is
\begin{equation}
  \boldsymbol{\mathcal{J}} = \frac{\partial \boldsymbol{\sigma}}{\partial \boldsymbol{\epsilon}} = \left(((1-c)^2(1-k) + k) \boldsymbol{\mathcal{P}}^+ + \boldsymbol{\mathcal{P}}^- \right) \boldsymbol{\mathcal{M}},
\end{equation}
where $\boldsymbol{\mathcal{M}}$ is the Jacobian_mult that was calculated by the constitutive model.

## Evolution Equation and History Variable

To avoid crack healing, a history variable $H$ is defined that is the maximum energy density over the
time interval $t=[0,t_0]$, where $t_0$ is the current time step, i.e.
\begin{equation}
H = \max_t (\Psi^{+})
\end{equation}

Now, the total free energy is redefined as:
\begin{equation}
\begin{aligned}
F =& \left[ (1-c)^2(1-k) + k \right] H +\Psi^{-} + \frac{g_c}{2l}c^2 + \frac{g_c l}{2} {|{\nabla c}|}^2 \\
  =& f_{loc} + \frac{g_c l}{2} {|{\nabla c}|}^2
\end{aligned}
\end{equation}
with
\begin{equation}
f_{loc} = \left[ (1-c)^2(1-k) + k \right] H +\Psi^{-} + \frac{g_c}{2l}c^2.
\end{equation}
Its derivatives are
\begin{equation}
\begin{aligned}
\frac{\partial f_{loc}}{\partial c} =& -2 (1-c)(1-k) H + 2 \frac{g_c}{2l} c\\
\frac{\partial^2 f_{loc}}{\partial c^2} =& 2 (1-k) H + 2 \frac{g_c}{2l}.
\end{aligned}
\end{equation}

The evolution equation for the damage parameter follows the Allen-Cahn equation
\begin{equation}
\dot{c} = -L \frac{\delta F}{\delta c} = -L \left( \frac{\partial f_{loc}}{\partial c} - \nabla \cdot \kappa \nabla c \right),
\end{equation}
where $L = (g_c \eta)^{-1}$ and $\kappa = g_c l$.

This equation follows the standard Allen-Cahn and thus can be implemented in MOOSE using the standard
Allen-Cahn kernels, TimeDerivative, AllenCahn, and ACInterface. There is now an action that automatically generates these kernels:
NonconservedAction. See the +PhaseField module documentation+ for more information.

## Example Input File Syntax

!listing modules/combined/test/tests/phase_field_fracture/crack2d_computeCrackedStress_smallstrain.i
         block=Materials/cracked_stress

!syntax parameters /Materials/ComputeCrackedStress

!syntax inputs /Materials/ComputeCrackedStress

!syntax children /Materials/ComputeCrackedStress
