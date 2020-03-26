# Compute Linear Elastic Phase Field Fracture Stress

## Description

This material implements the unified phase-field model for mechanics of damage and quasi-brittle failure from Jian-Ying Wu [!cite](JYWu2017). The pressure on the fracture surface can be optionally applied as described in [!cite](CHUKWUDOZIE2019957) and [!cite](Mikelic2019).

## Crack Surface Energy

The regularized functional $A_d$ is given as

\begin{equation}
A_d(d):= \int_{\mathcal{B}}\gamma(d,\nabla d) dV
\end{equation}

The crack surface density function $\gamma(d,\nabla d)$ is expressed in terms of the crack phase-field $d$ and its spatial gradient $\nabla d$ as
\begin{equation}
\gamma(d,\nabla d) = \frac{1}{c_0}(\frac{1}{l}\alpha(d) + l|\nabla d|^2)~~~~~~~\text{with}~~~~~~~c_0 = 4\int_0^1\sqrt{\alpha(\beta)}d\beta
\end{equation}
where the geometric function $\alpha(d)$ characterizes homogeneous evolution of the crack phase-field. $l$ is an internal length scale regularizing the sharp crack. $c_0$ is a scaling parameter such that the regularized functional $A_d(d)$ represents the crack surface.

The crack geometric function $\alpha(d)$ generally satisfies the following properties,
\begin{equation}
\alpha(0) = 0~~~~~~~~,~~~~~~~~\alpha(1) = 1
\end{equation}

In the classical phase-field models the modeling crack geometric function have been widely adopted for brittle fracture_energy
\begin{equation}
\alpha(d) = \begin{cases} d~~~~\text{Linear function} \\ d^2~~~\text{Quadratic function}\end{cases}
\end{equation}

## Elastic Energy

The elastic energy is defined as
\begin{equation}
\Psi(\varepsilon,d) = \omega(d)\Psi_0(\varepsilon)
\end{equation}

The monotonically decreasing energetic function $w(d)\in[0,1]$ describes degradation of the initial strain energy $\Psi_0(\varepsilon)$ as the crack phase-field evolves, satisfying the following properties [!cite](Miehe2015)
\begin{equation}
\omega'(d) < 0~~~~~\text{and}~~~~~\omega(0)=1,~~~~~\omega(1) = 0,~~~~~\omega'(1)=0
\end{equation}

The variation of the elastic energy gives constitutive relations
\begin{equation}
\boldsymbol{\sigma} = \omega(d)\frac{\partial{\Psi_0}}{\partial{\varepsilon}},~~~~~~~~Y=-\frac{\partial\Psi}{\partial d} = -\omega'(d)\mathcal{Y}
\end{equation}
where the thermodynamic force $Y$ drives evolution of the crack phase-field with the reference energy $\mathcal{Y}$ related to the strain field $\varepsilon$.

### Energetic Degradation Function

A genetic expression for degradation function $\omega(d)$ is given as
\begin{equation}
\omega(d) = \frac{1}{1+\phi(d)} = \frac{(1-d)^p}{(1-d)^p + Q(d)},~~~~~~\phi(d) = \frac{Q(d)}{(1-d)^p}
\end{equation}
for $p>0$ and continuous function Q(d) > 0. Jian-Ying Wu considers following polynomials
\begin{equation}
Q(d) = a_1d + a_1a_2d^2 + a_1a_2a_3d^3 + ...
\end{equation}
where the coefficients $a_i$ are calibrated from standard material properties.

The energetic function recovers some particular examples used in the literature, such as $\omega(d) = (1-d)^2$ when $p=2$ and $Q(d) = 1 - (1-d)^p$.

### Decomposition Approaches

The elastic energy is usually decomposed additively to distinguish between tensile and compressive contributions. Three decomposition approaches are implemented.

#### Strain Spectral Decomposition

The total strain energy density is defined as
\begin{equation}
\Psi = \omega(d) \Psi^{+} +\Psi^{-},
\end{equation}
where $\psi^{+}$ is the strain energy due to tensile stress, $\psi^{-}$ is the strain energy due to compressive stress.
\begin{equation}
\Psi^{\pm} = \lambda \left< \varepsilon_1 + \varepsilon_2 + \varepsilon_3 \right>^2_{\pm}/2 + \mu \left(\left< \varepsilon_1 \right>_{\pm}^2 + \left< \varepsilon_1 \right>_{\pm}^2 + \left< \varepsilon_1 \right>_{\pm}^2 \right),
\end{equation}
where $\epsilon_i$ is the $i$th eigenvalue of the strain tensor and $\left< \right>_{\pm}$ is an
operator that provides the positive or negative part.

To be thermodynamically consistent, the stress is related to the deformation energy density according
to
\begin{equation}
\boldsymbol{\sigma} = \frac{\partial \Psi}{\partial \boldsymbol{\varepsilon}}.
\end{equation}
Thus,
\begin{equation}
\boldsymbol{\sigma}^{\pm} = \frac{\partial \Psi^{\pm}}{\partial \boldsymbol{\varepsilon}} = \sum_{a=1}^3 \left( \lambda \left< \varepsilon_1 + \varepsilon_2 + \varepsilon_3 \right>_{\pm} + 2 \mu \left< \varepsilon_a \right>_{\pm} \right) \boldsymbol{n}_a \otimes \boldsymbol{n}_a,
\end{equation}
where $\boldsymbol{n}_a$ is the $a$th eigenvector.
The stress becomes
\begin{equation}
\boldsymbol{\sigma} = \left[(1-c)^2(1-k) + k \right] \boldsymbol{\sigma}^{+} - \boldsymbol{\sigma}^{-}.
\end{equation}

#### Strain Volumetric and Deviatoric Decomposition

The approach is based on the orthogonal decomposition of the linearized strain tensor in its spherical and deviatoric components:
\begin{equation}
\varepsilon = \varepsilon_S + \varepsilon_D,~~~~\varepsilon_D = \frac{1}{n}tr(\varepsilon)I,~~~~\varepsilon_D = \varepsilon - \frac{1}{n}tr(\varepsilon)I
\end{equation}
werhe $I$ deontes the n-dimensional identity tensor.

$\psi^{+}$ and $\psi^{-}$ is defined as
\begin{equation}
\psi^{+} = \frac{1}{2}\kappa\left<tr(\varepsilon)^2\right>_{+} + \mu\varepsilon_{D}\cdot\varepsilon_{D}
\end{equation}
\begin{equation}
\psi^{-} = \frac{1}{2}\kappa\left<tr(\varepsilon)^2\right>_{-}
\end{equation}

The stress is defined as
\begin{equation}
\boldsymbol{\sigma}^- = \kappa \left<tr(\varepsilon) \right>
\end{equation}
and
\begin{equation}
\boldsymbol{\sigma}^+ = \boldsymbol{\mathcal{C}}\varepsilon -\boldsymbol{\sigma}^-
\end{equation}

#### Stress Spectral Decomposition

$\psi^{+}$ and $\psi^{-}$ is defined as
\begin{equation}
\psi^{+} = \frac{1}{2} \boldsymbol{\sigma}^{+} : \boldsymbol{\varepsilon}
\end{equation}
\begin{equation}
\psi^{-} = \frac{1}{2} \boldsymbol{\sigma}^{-} : \boldsymbol{\varepsilon}.
\end{equation}

The compressive and tensile parts of the stress are computed from postive and negative projection tensors (computed from the spectral decomposition) according to
\begin{equation}
	\boldsymbol{\sigma}^+ = \mathbf{P}^+ \boldsymbol{\sigma}_0
\end{equation}
\begin{equation}
	\boldsymbol{\sigma}^- = \mathbf{P}^- \boldsymbol{\sigma}_0,
\end{equation}

To be thermodynamically consistent, the stress is related to the deformation energy density according
to
\begin{equation}
  \boldsymbol{\sigma} = \frac{\partial \psi_e}{\partial \boldsymbol{\varepsilon}} = \omega{d}\frac{\partial \psi^+}{\partial \boldsymbol{\varepsilon}} + \frac{\partial \psi^-}{\partial \boldsymbol{\varepsilon}}.
\end{equation}
Since
\begin{equation}
	\frac{\partial \psi^+}{\partial \boldsymbol{\varepsilon}} = \boldsymbol{\sigma}^+
\end{equation}
\begin{equation}
	\frac{\partial \psi^-}{\partial \boldsymbol{\varepsilon}} = \boldsymbol{\sigma}^-,
\end{equation}
then,
\begin{equation}
	\boldsymbol{\sigma} = \omega(d)\boldsymbol{\sigma}^+ + \boldsymbol{\sigma}^-
\end{equation}

The Jacobian matrix for the stress is
\begin{equation}
  \boldsymbol{\mathcal{J}} = \frac{\partial \boldsymbol{\sigma}}{\partial \boldsymbol{\varepsilon}} = \left((\omega(d) \boldsymbol{\mathcal{P}}^+ + \boldsymbol{\mathcal{P}}^- \right) \boldsymbol{\mathcal{C}},
\end{equation}
where $\boldsymbol{\mathcal{C}}$ is the elasticity tensor.


Note that stress spectral decomposition approach can be used for anisotropic elasticity tensor.

## Evolution Equation (Allen-Cahn)

To avoid crack healing, a history variable $H$ is defined that is the maximum energy density over the
time interval $t=[0,t_0]$, where $t_0$ is the current time step, i.e.
\begin{equation}
H = \max_t (\Psi^{+})
\end{equation}

Now, the total free energy is redefined as:
\begin{equation}
\begin{aligned}
F =& \omega(d) H +\Psi^{-} + \frac{1}{c_0}(\frac{g_c}{l}\alpha(d) + l|\nabla d|^2) \\
  =& f_{elastic} + f_{fracture} + \frac{g_c l}{c_0} {|{\nabla d}|}^2
\end{aligned}
\end{equation}
with
\begin{equation}
f_{loc} = f_{elastic} + f_{fracture}
\end{equation}
and
\begin{equation}
f_{fracture} = \frac{1}{c_0}\frac{g_c}{l}\alpha(d),~~~~~f_{elastic} = \omega(d)H +\Psi^{-}.
\end{equation}

Its derivatives are
\begin{equation}
\begin{aligned}
\frac{\partial f_{elastic}}{\partial d} =& \omega'(d)H\\
\frac{\partial^2 f_{elastic}}{\partial d^2} =& \omega''(d)H \\
\frac{\partial f_{fracture}}{\partial d} =& \frac{1}{c_0}\frac{g_c}{l} \alpha'(d) \\
\frac{\partial^2 f_{fracture}}{\partial d^2} =& \frac{1}{c_0}\frac{g_c}{l} \alpha''(d).
\end{aligned}
\end{equation}

To further avoid crack phase-field going to negative, $H$ should overcome a barrier energy. The barrier energy $f_{barrier}$ is determined by
\begin{equation}
f_{barrier} = -\frac{f_{fracture}}{\omega(d)}~/text{at}~d=0
\end{equation}
and the $H$ is modified as
\begin{equation}
H = \max_t (\Psi^{+}, f_{barrier})
\end{equation}

The evolution equation for the damage parameter follows the Allen-Cahn equation
\begin{equation}
\dot{d} = -L \frac{\delta F}{\delta d} = -L \left( \frac{\partial f_{loc}}{\partial d} - \nabla \cdot \kappa \nabla d \right),
\end{equation}
where $L = (g_c \tilde\eta)^{-1}$ and $\kappa = 2g_cl/c_0$.The $\tilde\eta = \eta/g_c$ is scaled by the $g_c$ which is consistent with the definition given by Miehe at.al [!cite](Miehe2015).

This equation follows the standard Allen-Cahn and thus can be implemented in MOOSE using the standard
Allen-Cahn kernels, TimeDerivative, AllenCahn, and ACInterface. There is now an action that automatically generates these kernels:
NonconservedAction. See the +PhaseField module documentation+ for more information.

## Pressure on the fracture surface

As suggested by [!cite](CHUKWUDOZIE2019957), the work of pressure forces acting along each side of the cracks that is added to the total free energy can be approximated by
\begin{equation}
\int_{\Gamma}p(\mathbf{u}\cdot \mathbf{n})ds \approx \int_{\Omega}p(\mathbf{u}\cdot\nabla d)
\end{equation}

Integration by parts yields
\begin{equation}
-\int_{\Omega}p(\mathbf{u}\cdot\nabla d) = \int_{\Omega}pd\nabla\cdot\mathbf{u}-\int_{\partial{\Omega}}pd(\mathbf{u}\cdot\mathbf{n})
\end{equation}

The boundary integral term can be neglected as in most applications $d=0$ on $\partial\Omega$. Some authors [!cite](Mikelic2019) have proposed to replace the indicator function $d$ with $d^2$ in the first term in order to make the functional convex. The indicator function is implemented as a generic material object that can be easily provided and modified in an input file.
The stress equilibrium and damage evolution equations are also modified to account for the pressure contribution.

## PETSc SNES variational inequalities solver option

Alternatively, the damage irreversibility condition can be enforced by using PETSc's SNES variational inequalities (VI) solver. In order to use PETSc's VI solver, upper and lower bounds for damage variable should be provided. Specifically, [`ConstantBoundsAux`](/ConstantBoundsAux.md) can be used to set the upper bound to be 1. [`VariableOldValueBoundsAux`](/VariableOldValueBoundsAux.md) can be used to set the lower bound to be the old value. Note that in order for these bounds to have an effect, the user has to specify the
PETSc options `-snes_type vinewtonssls` or `-snes_type vinewtonrsls`.

## Example Input File

!listing modules/combined/test/tests/phase_field_fracture/crack2d_iso.i
         block=Materials/damage_stress

## Example Input File with pressure

!listing modules/combined/test/tests/phase_field_fracture/crack2d_iso_with_pressure.i
         block=Materials/damage_stress

!listing modules/combined/test/tests/phase_field_fracture/crack2d_iso_with_pressure.i
				 block=Materials/pfbulkmat


!bibtex bibliography
