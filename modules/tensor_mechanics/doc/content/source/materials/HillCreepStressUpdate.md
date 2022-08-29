# Hill Creep Stress Update

!syntax description /Materials/ADHillCreepStressUpdate

## Description

This class computes a creep strain rate based on an equivalent deviatoric stress function that
is calculated as a function of Hill's function anisotropy parameters $F$, $G$, $H$, $L$, $M$, and $N$:
\begin{equation}
\tilde{q}(\sigma) = {[F(\sigma_{22} - \sigma_{33})^2 + G(\sigma_{33} - \sigma_{11})^2 + H(\sigma_{11} - \sigma_{22})^2
+ 2L\sigma_{23}^2 + 2M\sigma_{13}^2 + 2N\sigma_{12}^2]}^{1/2}
\end{equation}

The equivalent creep strain rate function may then be obtained as
\begin{equation}
\dot{\epsilon} = A_{aniso} (\tilde{q})^{n}
\end{equation}

where $A_{aniso}$ is a creep coefficient and $n$ the creep exponent.

The effective creep strain increment is obtained within the framework of a generalized (Hill plasticity) radial return mapping, see
[GeneralizedRadialReturnStressUpdate](/GeneralizedRadialReturnStressUpdate.md). This class computes the
generalized radial return inelastic increment.

More details on the Hill-type creep material model may be found in [!cite](stewart2011anisotropic).

### Numerical time integration error

The return mapping algorithm used to solve for elastic and inelastic strains relies on an additive decomposition. The stress increment can be determined, assuming a one-dimensional problem, as $\Delta\sigma = E \Delta\epsilon^{elastic} = E (\Delta\epsilon^{total} - \Delta\epsilon^{creep})$, where $E$ is a representative value of the Young's modulus. One way of limiting the error incurred in the numerical time integration is to compare it with the elastic strain increment during such an increment. Namely, we want to ensure that $err_{creep} \ll \frac{\Delta\sigma}{E}$. Estimating the creep numerical integration error as proportional to the increment of creep strain rates: $err_{creep} = (\Delta\dot{\epsilon}_{t+\Delta t} - \Delta\dot{\epsilon}_{t}) \cdot \Delta t$, the recommended time step is

\begin{equation}
\Delta t_{limit} = \frac{\Delta t \cdot E \cdot userTolerance}{\Delta\sigma},
\label{eq_num_time}
\end{equation}

where $\Delta t_{limit}$ is the computed time step limit, $\Delta t$ is the current time step, $\Delta\sigma$ denotes a stress time increment scalar, and $userTolerance$ is the maximum numerical time integration error selected by the user. As Eq. (1) suggests, the creep error depends on the material stiffness and the given stress increment. For complex simulation scenarios, controlling this error by only prescribing a maximum inelastic strain increment may not be sufficient to limit creep error. Furthermore, too conservative selection of the maximum inelastic strain may lead to a large increase in the number of time steps required to finish the simulation.

!alert warning title=Selection of time step increments with creep
It is recommended to choose a `max_inelastic_increment` that guarantees that creep strain increments are well within the small strain increment assumption. Complementarily, `max_integration_error` must be selected such that the numerical time integration will not significantly affect the creep results. Usually, for most applications, values on the order of $1.0\cdot10^{-3}$ - $1.0\cdot10^{-5}$ will suffice. For better results, these time integration limits may be combined with a soft [Terminator](/Terminator.md).

For three-dimensional problems, a norm of the stress diference and a representative value for the Young's modulus is used internally.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/ad_anisotropic_creep/ad_aniso_creep_y_3d.i block=Materials/trial_creep_two

!syntax parameters /Materials/ADHillCreepStressUpdate

!syntax inputs /Materials/ADHillCreepStressUpdate

!syntax children /Materials/ADHillCreepStressUpdate

!bibtex bibliography
