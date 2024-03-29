# Hill Creep Stress Update

!syntax description /Materials/ADHillCreepStressUpdate

## Description

This class computes a creep strain rate based on an equivalent deviatoric stress function (${\sigma_e}$) that
is calculated as a function of Hill's function anisotropy parameters $F$, $G$, $H$, $L$, $M$, and $N$:
\begin{equation}
\sigma_e = \tilde{q}(\boldsymbol{\sigma}) = {[F(\sigma_{22} - \sigma_{33})^2 + G(\sigma_{33} - \sigma_{11})^2 + H(\sigma_{11} - \sigma_{22})^2
+ 2L\sigma_{23}^2 + 2M\sigma_{13}^2 + 2N\sigma_{12}^2]}^{1/2}
\label{hill_stress_function}
\end{equation}

where $\boldsymbol{\sigma}$ and $\sigma_{ij}$ are the stress tensor and its component, respectively. The [!eqref](hill_stress_function) is also called Hill stress function and accounts for anisotropy through the anisotropy parameters. The equivalent creep strain rate function may then be obtained as
\begin{equation}
\dot{\epsilon} = A {\sigma_e}^{n}
\label{creep_law}
\end{equation}

where $A$ is a creep coefficient and $n$ the creep exponent.

The effective creep strain increment is obtained within the framework of a generalized (Hill plasticity) radial return mapping, see
[GeneralizedRadialReturnStressUpdate](/GeneralizedRadialReturnStressUpdate.md). This class computes the
generalized radial return inelastic increment. More details on the Hill-type creep material model may be found in [!cite](stewart2011anisotropic).


### Creep strain integration scheme

Newton iteration is performed for computing the effective creep strain increment $\Delta \gamma$ with increment in the $\Delta \gamma$ (d$\Delta \gamma$) for each Newton iteration computed as:

\begin{equation}
d{\Delta \gamma}_{n+1} = - \frac{R({\Delta \gamma}_n)}{R'({\Delta \gamma}_n)} \\
{\Delta \gamma}_{n+1} = {\Delta \gamma}_{n} + d{\Delta \gamma}_{n+1}
\label{newtons_iteration}
\end{equation}

where $R$ is the residual and $R'$ is the derivative of residual with respect to the ${\Delta \gamma}$. The residual and its derivative are computed as:

\begin{equation}
R = {\dot{\epsilon_c}} \times \Delta t - \Delta \gamma
\label{residual}
\end{equation}

\begin{equation}
\frac{\partial R}{\partial \Delta \gamma} = \frac{\partial {\dot{\epsilon_c}}} {\Delta \gamma} \times \Delta t - 1.0
\label{residual_derivative}
\end{equation}

Substituting [!eqref](creep_law) in [!eqref](residual) and [!eqref](residual_derivative), we obtain:

\begin{equation}
R = A {\mathbf{\sigma_e}}^n \times \Delta t - \Delta \gamma
\label{residual2}
\end{equation}

\begin{equation}
\frac{\partial R}{\partial \Delta \gamma} = A n {\sigma_e}^{n-1} \frac{\partial \sigma_e}{\partial \Delta{\gamma}} \times \Delta t - 1.0
\label{residual_derivative2}
\end{equation}

We need expressions for $\sigma_e$ and $\frac {\partial \sigma_e} {\partial \Delta{\gamma}}$ in terms of trial stress $\mathbf{\sigma^{tr}}$ and $\Delta \gamma$, which are then substituted in [!eqref](residual2) and [!eqref](residual_derivative2).

#### Isotropic Elasticity

\begin{equation}
\sigma_e = {\sigma^{tr}_e} - 3G\Delta \gamma
\label{isotropic_sigma_e}
\end{equation}

\begin{equation}
\frac{\partial \sigma_e}{\partial \Delta \gamma} = - 3G
\label{isotropic_sigma_e_derivative}
\end{equation}

where $G$ is the shear modulus. For details of [!eqref](isotropic_sigma_e) and [!eqref](isotropic_sigma_e_derivative) see [!cite](dunne2005introduction).

#### Anisotropic Elasticity

For cases with anisotropic elasticity [!eqref](isotropic_sigma_e) is not valid. The stress tensor after radial return for the case with anisotropic elasticity is expressed as:

\begin{equation}
\boldsymbol{\sigma} = \boldsymbol{\sigma^{tr}} - \boldsymbol{C} \Delta \boldsymbol{\epsilon_c}
\label{anisotropic_sigma}
\end{equation}

where $\boldsymbol{\epsilon_c}$ is the creep strain tensor and $\boldsymbol{C}$ is the elasticity tensor. Rewriting [!eqref](hill_stress_function):

\begin{equation}
\sigma_e = \tilde{q}(\boldsymbol{\sigma})
\label{anisotropic_sigma_e}
\end{equation}

\begin{equation}
\frac{\partial \sigma_e}{\partial \Delta{\gamma}} = \frac{\partial \tilde{q}(\boldsymbol{\sigma^{tr}} - \boldsymbol{C} \Delta \boldsymbol{\epsilon_c})}{\partial \Delta \gamma} = \frac{\partial \tilde{q}(\boldsymbol{\sigma^{tr}} - \boldsymbol{C} \Delta \boldsymbol{\epsilon_c})}{\partial \Delta \boldsymbol{\epsilon_c}} \frac{\partial \Delta \boldsymbol{\epsilon_c}}{\partial \Delta{\gamma}}
\label{anisotropic_sigma_e_derivative}
\end{equation}

Note that [!eqref](anisotropic_sigma_e_derivative) uses chain rule. Normality hypothesis is expressed as:

\begin{equation}
\Delta \boldsymbol{\epsilon_c} = \Delta \gamma \frac{\partial \tilde{q}}{\partial \boldsymbol{\sigma}}
\label{normality_hypothesis}
\end{equation}

and the last term of [!eqref](anisotropic_sigma_e_derivative) is obtained by taking derivative of [!eqref](normality_hypothesis) as:

\begin{equation}
\frac{\partial \Delta \boldsymbol{\epsilon_c}}{\partial \Delta{\gamma}} = \frac{\partial \tilde{q}}{\partial \boldsymbol{\sigma}}
\label{normality_hypothesis_derivative}
\end{equation}

### Numerical time integration error

The return mapping algorithm used to solve for elastic and inelastic strains relies on an additive decomposition. The stress increment can be determined, assuming a one-dimensional problem, as $\Delta\sigma = E \Delta\epsilon^{elastic} = E (\Delta\epsilon^{total} - \Delta\epsilon^{creep})$, where $E$ is a representative value of the Young's modulus. One way of limiting the error incurred in the numerical time integration is to compare it with the elastic strain increment during such an increment. Namely, we want to ensure that $err_{creep} \ll \frac{\Delta\sigma}{E}$. Estimating the creep numerical integration error as proportional to the increment of creep strain rates: $err_{creep} = (\Delta\dot{\epsilon}_{t+\Delta t} - \Delta\dot{\epsilon}_{t}) \cdot \Delta t$, the recommended time step is

\begin{equation}
\Delta t_{limit} = \frac{\Delta t \cdot E \cdot userTolerance}{\Delta\sigma},
\label{eq_num_time}
\end{equation}

where $\Delta t_{limit}$ is the computed time step limit, $\Delta t$ is the current time step, $\Delta\sigma$ denotes a stress time increment scalar, and $userTolerance$ is the maximum numerical time integration error selected by the user. As Eq. (1) suggests, the creep error depends on the material stiffness and the given stress increment. For complex simulation scenarios, controlling this error by only prescribing a maximum inelastic strain increment may not be sufficient to limit creep error. Furthermore, too conservative selection of the maximum inelastic strain may lead to a large increase in the number of time steps required to finish the simulation.

!alert warning title=Selection of time step increments with creep
It is recommended to choose a `max_inelastic_increment` that guarantees that creep strain increments are well within the small strain increment assumption. Complementarily, `max_integration_error` must be selected such that the numerical time integration will not significantly affect the creep results. Usually, for most applications, values on the order of $1.0\cdot10^{-3}$ - $1.0\cdot10^{-5}$ will suffice. For better results, these time integration limits may be combined with a soft [Terminator](/Terminator.md).

For three-dimensional problems, a norm of the stress difference and a representative value for the Young's modulus is used internally.

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/ad_anisotropic_creep/ad_aniso_creep_y_3d.i block=Materials/trial_creep_two

!syntax parameters /Materials/ADHillCreepStressUpdate

!syntax inputs /Materials/ADHillCreepStressUpdate

!syntax children /Materials/ADHillCreepStressUpdate

!bibtex bibliography
