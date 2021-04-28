# Phase-Field Model for Fracture

## Description

Phase-field model for fracture is one of the most widely-adopted variational approach to fracture. This approach is attractive due to its potential for capturing the evolution of complex crack patterns without the need for algorithmic crack-front tracking methods.

The phase-field model for fracture introduces a length scale and non-locality, which helps to address the well-known mesh dependence of purely local damage models, by regularizing sharp crack surfaces using Allen-Cahn approximation. In its simplest form, where the system only concerns about small deformation and fracture, the model describes the minimization of the potential:
\begin{equation}
  \begin{aligned}
    \min_{\boldsymbol{u},c} \ &\int\limits_\Omega \psi^e(\boldsymbol{\varepsilon}(\boldsymbol{u}),c) \text{ d}V + \int\limits_\Omega \psi^f(c, \nabla c) \text{ d}V - \int\limits_\Omega \boldsymbol{b}\cdot\boldsymbol{u} \text{ d}V - \int\limits_{\partial\Omega_t} \boldsymbol{t}\cdot\boldsymbol{u} \text{ d}A,\\
    & \text{subject to } \ \dot{c} \geqslant 0,
  \end{aligned}
\end{equation}
where $\boldsymbol{u}$ is the displacement, $c$ is the phase field (also interpreted as damage). $\psi^e(\boldsymbol{u},c)$ is the strain energy density, $\psi^f(c, \nabla c)$ is the regularized fracture energy density, and $\boldsymbol{b}$ and $\boldsymbol{t}$ are respectively the body force and the traction describing external work.

For large deformation and coupling with other physics, refer to the following pages:

- [Large deformation phase-field model for fracture (TODO)](/PhaseFieldFracture.md)
- [Dynamic fracture (TODO)](/PhaseFieldFracture.md)
- [Pressurized fracture and hydraulic fracture (TODO)](/PhaseFieldFracture.md)
- [Thermomechanical-fracture coupling (TODO)](/PhaseFieldFracture.md)
- [Creep and plasticity coupling (TODO)](/PhaseFieldFracture.md)

## Governing equations

The minimization with respect to $\boldsymbol{u}$ leads to the balance of linear momentum:
\begin{equation}
  \begin{aligned}
    \nabla \cdot \boldsymbol{\sigma} + \boldsymbol{b} = \boldsymbol{0}, &\quad \forall \boldsymbol{X} \in \Omega, \\
    \boldsymbol{\sigma} \boldsymbol{n} = \boldsymbol{t}, &\quad \forall \boldsymbol{X} \in \partial\Omega_t.
  \end{aligned}
\end{equation}
Following the Coleman-Noll procedure, the thermodynamic conjugate to $\boldsymbol{\varepsilon}(\boldsymbol{u})$ is defined as $\boldsymbol{\sigma} = \psi^e_{,\boldsymbol{\varepsilon}}$, with comma denoting the partial derivative. Minimization of the potential with respect to $c$ invokes the Karush-Kuhn-Tucker conditions:
\begin{equation}
  -\phi^f = -\nabla \cdot \boldsymbol{\xi} + f^\text{en} \geqslant 0, \quad \dot{c} \geqslant 0, \quad \phi^f\dot{c} = 0,
\end{equation}
where $\boldsymbol{\xi}$ is the thermodynamic conjugate to $\nabla c$, and $f^\text{en}$ is the thermodynamic conjugate to $c$. The specific constitutive choices are discussed below.

## Constitutive choices

The specific forms of the strain energy density and the fracture energy density are presented below.

### Strain energy

The strain energy density typically employs some form of decomposition to split the energy into an active part (that couples with fracture) and an inactive part, i.e. $\psi^e = \omega(c) \psi^{e^+}(\boldsymbol{\varepsilon}) + \psi^{e^-}(\boldsymbol{\varepsilon})$ where $\omega(c)$ is the degradation function to be discussed momentarily. A variety of strain energy density definitions are available in the literature. We have implemented several commonly used forms of the strain energy density:

- [The strain energy density without split](PhaseFieldFractureNoSplit.md)
- [The strain-based spectral split](PhaseFieldFractureStrainSpectralSplit.md)
- [The strain-based volumetric-deviatoric split](PhaseFieldFractureStrainVolDevSplit.md)
- [The stress-based spectral split](PhaseFieldFractureStressSpectralSplit.md)

### Fracture energy

We employ the Allen-Cahn approximation of the fracture energy density:
\begin{equation}
  \begin{aligned}
    &\psi^f = \int\limits_\Omega L \left[ \alpha(c) + \frac{1}{2} \nabla c \cdot \kappa \nabla c \right] \text{ d}V, \\
    &c_0 = 4\int\limits_0^1\sqrt{\alpha(\beta)} \text{ d}\beta, \quad L = \frac{\mathcal{G}_c}{c_0l}, \quad \kappa = 2l^2,
  \end{aligned}
\end{equation}
where $\mathcal{G}_c$ is the critical energy release rate, $l$ is the phase-field regularization length, and $\alpha(c)$ is the crack geometric function. $L$ and $\kappa$ are the mobility and the interface coefficient in a traditional phase-field context. The thermodynamic conjugates are $\boldsymbol{\xi} = L\kappa\nabla c$ and $f^\text{en} = L\alpha_{,c} + \omega_{,c}\psi^{e^+}$.

[!cite](JYWu2017) derived a series of combinations of the crack geometric function and the degradation function that recovers a family of traction-separation laws. Most commonly used ones are
\begin{equation}
  \alpha = c^2, \quad \omega(c) = (1-c)^2
\end{equation}
for brittle fracture, and
\begin{equation}
  \alpha = 2c-c^2, \quad \omega(c) = \frac{(1-c)^2}{(1-c)^2+mc(1-c/2)}, \quad m = \frac{2L}{\psi_c}
\end{equation}
for cohesive fracture with a linear traction-separation law, where $\psi_c$ is the critical fracture energy.

## Solution strategies

PETSc's variational inequality solver is used to enforce the bound constraint. Several solution schemes are provided to solve this numerically challenging softening system. The hybrid formation is also discussed below.

### Enforcing the bound constraint

In the discrete form, the damage irreversibility $\dot{c} \geqslant 0$ reads as $c \geqslant \bar{c}$ where $\bar{c}$ is the damage from the previously converged step. Also, in practice, it is important to guard against damage from growing above 1 to ensure well-posedness of the constitutive assumptions. Hence, it is preferred to also apply an upper bound on $c$ as $c \leqslant 1$. These conditions can be enforced using PETSc's SNES variational inequality (VI) solver.

In order to use PETSc's VI solver, vectors representing the upper and lower bound for the damage variable should be provided. In particular, [`ConstantBoundsAux`](/ConstantBoundsAux.md) can be used to set the upper bound to be 1, and [`VariableOldValueBoundsAux`](/VariableOldValueBoundsAux.md) can be used to set the lower bound to $\bar{c}$.

!listing modules/combined/examples/phase_field_fracture/brittle_mode1.i
         block=Bounds
         caption=Example for specifying the bound constraints

!alert note
In order for the bounds to take effect, the user has to specify the PETSc options `-snes_type vinewtonssls` or `-snes_type vinewtonrsls`. The latter is recommended.

### The monolithic solution scheme

Let us denote the time step using subscript $n$, the displacement subproblem as $\mathcal{D}(\boldsymbol{u},c) = 0$, and the fracture subproblem as $\mathcal{F}(\boldsymbol{u},c) = 0$. Given solution from the previous time step, $\boldsymbol{u}_n$ and $c_n$, the monolithic solution scheme can be written as
\begin{equation}
  \begin{aligned}
    \begin{cases}
      \mathcal{D}(\boldsymbol{u}_{n+1},c_{n+1}) = 0, \\
      \mathcal{F}(\boldsymbol{u}_{n+1},c_{n+1}) = 0, \\
      c_n \leqslant c_{n+1} \leqslant 1.
    \end{cases}
  \end{aligned}
\end{equation}
In other words, the system contains DoFs for both the displacements and the damage, and the deformation and the fracture are fully coupled. This system is unconditionally stable (in terms of time discretization) but is very numerically challenging especially in the regime of unstable crack propagation.

### The semi-monolithic solution scheme

The semi-monolithic solution scheme addresses some of the issues with the monolithic scheme, by replacing the active elastic energy $\psi^{e^+}$ with the values from the previous time step. The semi-monolithic solution scheme can be written as
\begin{equation}
  \begin{aligned}
    \begin{cases}
      \mathcal{D}(\boldsymbol{u}_{n+1},c_{n+1}) = 0, \\
      \mathcal{F}(\boldsymbol{u}_{n},c_{n+1}) = 0, \\
      c_n \leqslant c_{n+1} \leqslant 1.
    \end{cases}
  \end{aligned}
\end{equation}
To use this solution scheme, simply set `use_old_elastic_energy = true` in the damage model.

### The one-pass staggered solution scheme

Another widely-adopted approach similar to the semi-monolithic scheme is the one-pass staggered solution scheme, where the displacements are first solved with a "frozen" phase field, then the phase field is updated with the "frozen" new displacements. The scheme can be written as
\begin{equation}
  \begin{aligned}
    &\begin{cases}
      \mathcal{D}(\boldsymbol{u}_{n+1},c_n) = 0,
    \end{cases}\\
    &\begin{cases}
      \mathcal{F}(\boldsymbol{u}_{n+1},c_{n+1}) = 0, \\
      c_n \leqslant c_{n+1} \leqslant 1.
    \end{cases}
  \end{aligned}
\end{equation}
This solution scheme can be implemented using the [MultiApp System](MultiApps/index.md).

!alert warning
Although numerically appealing, the solution obtained using the semi-monolithic solution scheme or the one-pass staggered solution scheme only approaches the exact solution when the time step size is sufficiently small.

### The alternating minimization solution scheme

The alternating minimization scheme extends the one-pass staggered solution scheme by iteratively solving the two subproblems until convergence. This solution scheme can be implemented using [Picard iterations](Executioner/index.md). Denoting the picard iteration using superscript $p$, the alternating minimization solution scheme can be written as
\begin{equation}
  \begin{aligned}
    &\text{Set } p \leftarrow 0, \quad \boldsymbol{u}_{n+1}^0 \leftarrow \boldsymbol{u}_n, \quad c_{n+1}^0 \leftarrow c_n. \\
    &\text{Repeat }\\
    &\quad\begin{cases}
      \mathcal{D}(\boldsymbol{u}_{n+1}^{p+1},c_{n+1}^p) = 0,
    \end{cases}\\
    &\quad\begin{cases}
      \mathcal{F}(\boldsymbol{u}_{n+1}^{p+1},c_{n+1}^{p+1}) = 0, \\
      c_n \leqslant c_{n+1}^{p+1} \leqslant 1,
    \end{cases}\\
    &\quad\text{Set } p \leftarrow p+1,\\
    &\text{Until convergence.}\\
    &\text{Set } \boldsymbol{u}_{n+1} \leftarrow \boldsymbol{u}_{n+1}^p, \quad c_{n+1} \leftarrow c_{n+1}^p.
  \end{aligned}
\end{equation}

### The hybrid formulation

In general, the split of strain energy density is applied consistently in the displacement subproblem as well as in the fracture subproblem. In other words, compression transmission across crack surfaces are permitted. However, in certain scenarios, it is known a-prioi that the system is tension, hence the split of strain energy density is not necessary in the displacement subproblem, resulting in a potential saving in the computational cost. To employ such hybrid formulation, simply set the parameter `hybrid = true` in the damage model, and an isotropic degradation will be applied on the stress.

## Example Input Files

#### Benchmark problems:

- Mode-I, brittle fracture, semi-monolithic solution scheme [input](modules/combined/examples/phase_field_fracture/brittle_mode1.i)
- Mode-II, brittle fracture, semi-monolithic solution scheme [input](modules/combined/examples/phase_field_fracture/brittle_mode2.i)
- Mode-I, cohesive fracture, semi-monolithic solution scheme [input](modules/combined/examples/phase_field_fracture/cohesive_mode1.i)
- Mode-II, cohesive fracture, semi-monolithic solution scheme [input](modules/combined/examples/phase_field_fracture/cohesive_mode2.i)
- Mode-II, cohesive fracture, alternating minimization solution scheme [input 1](modules/combined/examples/phase_field_fracture/cohesive_mode2_picard_damage.i) and [input 2](modules/combined/examples/phase_field_fracture/cohesive_mode2_picard_disp.i)


!bibtex bibliography
