# SpecificImpulse1Phase

!syntax description /Postprocessors/SpecificImpulse1Phase

# Discussion

Specific impulse is a measure of how much thrust is produced by an exhaust gas
per mass flow rate. Specific impulse is denoted by $I_{sp}$ and defined as:

\begin{equation}
  I_{sp} = \frac{F}{g \dot{m}},
\end{equation}

where $g = 9.81 \text{m}/\text{s}^2$, $F$ is thrust, and $\dot{m}$ is the mass flow rate.
Specific impulse has units of seconds. This equation can be generalized to an averaged $I_{sp}$
over a transient burn where the quantities on the right hand side may depend on time.

\begin{equation}
  I_{sp} = \frac{\int_0^T F(t) dt}{g \int_0^T \dot{m}(t) dt}.
\end{equation}

The postprocessor allows to switch between instantaneous and averaged $I_{sp}$ using the `accumulative` parameter.

The postprocessor computes $I_{sp}$ by assuming an isentropic change of state from the conditions on the boundary
to outlet conditions. The outlet pressure must be provided to parameter `p_exit`. It is assumed that the expansion
is perfect:

1. The outlet area expands the gas exactly to `p_exit`. If the ambient pressure is not equal to the resulting pressure from the
   expansion characteristics of the nozzle, a real nozzle would experience a shock that reduces its efficiency. The possible occurrence of a shock is neglected.

2. Losses and heat transfer are neglected.

!syntax parameters /Postprocessors/SpecificImpulse1Phase

!syntax inputs /Postprocessors/SpecificImpulse1Phase

!syntax children /Postprocessors/SpecificImpulse1Phase

!bibtex bibliography
