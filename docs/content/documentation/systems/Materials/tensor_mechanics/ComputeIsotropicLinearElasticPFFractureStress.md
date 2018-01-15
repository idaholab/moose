# Compute Isotropic Linear Elastic Phase Field Fracture Stress
!syntax description /Materials/ComputeIsotropicLinearElasticPFFractureStress

##Description
This material implements the phase field fracture model from Chakraborty et al., calculating the stress and the free energy derivatives required for the model. It works with the standard phase field kernels for nonconserved variables. In the model, a nonconserved order parameter $c$ defines the crack, where $c = 0$ in undamaged material and $c = 1$ in cracked material. Cracked material can sustain a compressive stress, but not a tensile one. $c$ evolves to minimize the elastic free energy of the system.

This model assumes linear elastic mechanical deformation with an isotropic elasticity tensor, where $\lambda$ and $\mu$ are the first and second Lam&egrave; constants.

###Free energy definition
The total strain energy density is defined as
\begin{equation}
\Psi = [(1-c)^2(1-k) + k] \Psi^{+} +\Psi^{-},
\end{equation}
where $\psi^{+}$ is the strain energy due to tensile stress, $\psi^{-}$ is the strain energy due to compressive stress, and $k \ll 0$ is a parameter used to avoid non-positive definiteness at or near complete damage.
\begin{equation}
\Psi^{\pm} = \lambda \left< \epsilon_1 + \epsilon_2 + \epsilon_3 \right>^2_{\pm}/2 + \mu \left(\left< \epsilon_1 \right>_{\pm}^2 + \left< \epsilon_1 \right>_{\pm}^2 + \left< \epsilon_1 \right>_{\pm}^2 \right),
\end{equation}
where $\epsilon_i$ is the $i$th eigenvalue of the strain tensor and $\left< \right>_{pm}$ is an operator that provides the positive or negative part.

The crack energy density is defined as
\begin{equation}
\gamma = g_c \frac{1}{2l}c^2 + g_c \frac{l}{2} {| \nabla c |}^2,
\end{equation}
where $l$ is the width of the crack interface and $g_c$ is a parameter related to the energy release rate.

The total local free energy density is defined as
\begin{equation}
\begin{aligned}
F &=& \Psi + \gamma \\
&=&[(1-c)^2(1-k) + k] \Psi^{+} +\Psi^{-} + \frac{g_c}{2l}c^2 + \frac{g_c l}{2} {|{\nabla c}|}^2.
\end{aligned}
\end{equation}

###Stress definition
To be thermodynamically consistent, the stress is related to the deformation energy density according to
\begin{equation}
\mathbf{\sigma} = \frac{\partial \Psi}{\partial \mathbf{\epsilon}}.
\end{equation}
Thus,
\begin{equation}
\mathbf{\sigma}^{\pm} = \frac{\partial \Psi^{\pm}}{\partial \mathbf{\epsilon}} = \sum_{a=1}^3 \left( \lambda \left< \epsilon_1 + \epsilon_2 + \epsilon_3 \right>_{\pm} + 2 \mu \left< \epsilon_a \right>_{\pm} \right) \mathbf{n}_a \otimes \mathbf{n}_a,
\end{equation}
where \mathbf{n}_a is the $a$th eigenvector.
The stress becomes
\begin{equation}
\mathbf{\sigma} = \left[(1-c)^2(1-k) + k \right] \mathbf{\sigma}^{+} - \mathbf{\sigma}^{-}.
\end{equation}

###Evolution equation and History variable
To avoid crack healing, a history variable $H$ is defined that is the maximum energy density over the time interval $t=[0,t_0]$, where $t_0$ is the current time step, i.e.
\begin{equation}
H = \max_t (\Psi^{+})
\end{equation}

Now, the total free energy is redefined as:
\begin{equation}
\begin{aligned}
F &=& \left[ (1-c)^2(1-k) + k \right] H +\Psi^{-} + \frac{g_c}{2l}c^2 + \frac{g_c l}{2} {|{\nabla c}|}^2 \\
&=& f_{loc} + \frac{g_c l}{2} {|{\nabla c}|}^2
\end{aligned}
\end{equation}
with
\begin{equation}
f_{loc} = \left[ (1-c)^2(1-k) + k \right] H +\Psi^{-} + \frac{g_c}{2l}c^2.
\end{equation}
Its derivatives are
\begin{equation}
\begin{aligned}
\frac{\partial f_{loc}}{\partial c} &=& -2 (1-c)(1-k) H + 2 \frac{g_c}{2l} c\\
\frac{\partial^2 f_{loc}}{\partial c^2} &=& 2 (1-k) H + 2 \frac{g_c}{2l}.
\end{aligned}
\end{equation}

The evolution equation for the damage parameter follows the Allen-Cahn equation
\begin{equation}
\dot{c} = -L \frac{\delta F}{\delta c} = -L \left( \frac{\partial f_{loc}}{\partial c} - \nabla \cdot \kappa \nabla c \right),
\end{equation}
where $ L = (g_c \eta)^{-1}$ and $\kappa = g_c l$.

This equation follows the standard Allen-Cahn and thus can be implemented in MOOSE using the standard Allen-Cahn kernels, [TimeDerivative](/TimeDerivative.md), [AllenCahn](/AllenCahn), and [ACInterface](/ACInterface). There is now an action that automatically generates these kernels: NonconservedAction.

## Example Input File Syntax
!listing modules/combined/test/tests/phase_field_fracture/void2d_iso.i block=Materials/pf_elastic_energy

!syntax parameters /Materials/ComputeIsotropicLinearElasticPFFractureStress

!syntax inputs /Materials/ComputeIsotropicLinearElasticPFFractureStress

!syntax children /Materials/ComputeIsotropicLinearElasticPFFractureStress
