# Brayton Cycle

## Introduction

The Brayton cycle is the ideal thermodynamic cycle for gas-turbine engines.
Usually, it is used in an *open* system, where gas is drawn in from the environment
and exhausted back into the environment. However, for some analyses, it is simpler
to analyze the system as if it were a *closed* system that cycles the same
working fluid in a closed loop, which can be done without significant loss of
accuracy. Here we discuss how both open and closed cycles can be modeled.

In its simplest form, a Brayton cycle requires the following elements:

- A *heat source* to add thermal energy to the working gas,
- A *heat sink* to remove thermal energy from the working gas (in an open system, exhaust into the environment serves this function),
- A *compressor* to compress the working gas, and
- A *turbine* to expand the working gas.

Usually, the compressor and turbine are operated on the same *shaft*. In steady operation,
the work the turbine provides to the shaft is partially consumed by the compressor
when it applies work to the working fluid. The remainder can be used for the
application's purposes, like electricity generation via a *generator* on the shaft.
In practice, when starting up a Brayton engine, the turbine is not yet applying
work to the shaft, and thus the compressor is not functioning either. Thus,
a *motor* is used to drive the shaft until the cycle can sustain itself.

## Example

Now an example will be given, first discussing the commonalities between the
open and closed cycles and then describing the differences in the following
sections.

The turbo-machinery setup for this example consists of a shaft, motor, compressor,
turbine, and generator. The [Shaft.md] component connects them and specifies
the initial shaft speed (here equal to zero since we are starting from rest):

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/open_brayton_cycle.i
         block=Components/shaft
         link=False

As mentioned previously, when starting up, a motor is needed, so we use
[ShaftConnectedMotor.md]:

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/open_brayton_cycle.i
         block=Components/motor
         link=False

The `torque` parameter is set to an arbitrary value, since this parameter is controlled
using the control logic system:

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/open_brayton_cycle.i
         block=ControlLogic
         link=False

The function chosen here is a linear ramp up from zero and back down to zero,

!equation
\tau_\text{motor}(t) = \left\{\begin{array}{l l}
  \frac{\tau_\text{motor,max}}{t_1} t & 0 \leq t \leq t_1\\
  \tau_\text{motor,max} - \frac{\tau_\text{motor,max}}{t_2 - t_1}(t - t_1) & t_1 \leq t \leq t_2\\
  0 & t > t_2\\
  \end{array}\right. \eqp

This is accomplished with a [PiecewiseLinear.md] function:

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/open_brayton_cycle.i
         block=Functions/motor_torque_fn
         link=False

The compressor and turbine components are modeled using [ShaftConnectedCompressor1Phase.md]:

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/open_brayton_cycle.i
         block=Components/compressor
         link=False

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/open_brayton_cycle.i
         block=Components/turbine
         link=False

The pressure ratio curves are adapted from [!cite](guillen2020). In this reference,
these curves are given as $r_p(\dot{m}, \omega)$, rather than $r_p(\nu, \alpha)$,
so the curves were processed beforehand. As is often the case, there was data
needed for these curves that was not available. For example,

!equation
\nu \equiv \frac{ (\frac{\dot{m}}{\rho_{\text{0, in}} c_{\text{0, in}}}) }
  { (\frac{\dot{m}}{\rho_{\text{0, in}} c_{\text{0, in}}})_{\text{rated}} } \eqc

but values for $\rho_0$ and $c_0$ are not known, only a rated mass flow rate
$\dot{m}_\text{rated}$ was known, so $\nu$ was approximated as

!equation
\nu \approx \frac{\dot{m}}{\dot{m}_\text{rated}} \eqc

which assumes $\rho_{\text{0, in}} c_{\text{0, in}} \approx (\rho_{\text{0, in}} c_{\text{0, in}})_\text{rated}$.
Similarly,

!equation
\alpha \approx \frac{\omega}{\omega_\text{rated}} \eqp

The resulting curves are shown in [rp_vs_mfr]:

!media thermal_hydraulics/modeling_guide/brayton_cycle/rp_vs_mfr.png
       id=rp_vs_mfr
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Pressure ratio curves for the compressor and turbine.

The efficiencies of both the compressor and turbine were set to constant values.

Finally, a generator is modeled using a [ShaftConnectedMotor.md] by using a
negative torque of the form

!equation
\tau_\text{gen}(\omega) = -a \omega \eqc

where $a$ is a positive value. This is accomplished as follows:

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/open_brayton_cycle.i
         block=Components/generator
         link=False

with the following function:

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/open_brayton_cycle.i
         block=Functions/generator_torque_fn
         link=False

### Closed Brayton Cycle

The model discussed in this section can be found in the following input file:
[closed_brayton_cycle.i](test/tests/problems/brayton_cycle/closed_brayton_cycle.i).

In the closed cycle, a strong convection source term is applied to bring the
working fluid back to near the ambient temperature:

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/closed_brayton_cycle.i
         block=Components/cooling
         link=False

### Open Brayton Cycle

The model discussed in this section can be found in the following input file:
[open_brayton_cycle.i](test/tests/problems/brayton_cycle/open_brayton_cycle.i).

In the open cycle, instead of a heat sink, there is an *inlet* connected
to the compressor inlet piping, and an *outlet* connected to the turbine outlet
piping. For the inlet, the stagnation pressure and temperature $(p_0, T_0)$ formulation
is appropriate, since the environment can be approximated to be at rest. The
ambient pressure and temperature can then be used for $p_0$ and $T_0$, respectively:

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/open_brayton_cycle.i
         block=Components/inlet
         link=False

There is a single outlet formulation, which specifies the ambient pressure:

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/open_brayton_cycle.i
         block=Components/outlet
         link=False

### Results

[shaft_speed_vs_time_open] and [shaft_speed_vs_time_closed] show the shaft speed
transient for the open and closed cycles, respectively. The shaft speed profile
indicates that the motor torque is fairly dominant in this example, as the profile
appears to be an integral of the motor torque profile that was used; the motor
torque was ramped up from zero to a max at $t=t_1$ and then back down to zero
at $t=t_2$. The profile seen here may not necessarily be realistic, for several reasons,
including the following:

- Moments of inertia of the various shaft components were chosen arbitrarily,
- The motor torque profile was chosen arbitrarily,
- The generator torque relationship was chosen arbitrarily, and
- Friction losses on the shaft were omitted.

!col! small=6 medium=6 large=6

!media thermal_hydraulics/modeling_guide/brayton_cycle/shaft_speed_vs_time_open.png
       id=shaft_speed_vs_time_open
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Shaft speed transient for the open cycle.

!col-end!

!col! small=6 medium=6 large=6

!media thermal_hydraulics/modeling_guide/brayton_cycle/shaft_speed_vs_time_closed.png
       id=shaft_speed_vs_time_closed
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Shaft speed transient for the closed cycle.

!col-end!

[mfr_vs_time_open] and [mfr_vs_time_closed] show the mass flow rate
transient for the open and closed cycles, respectively. At the beginning of the
transient, the mass flow rate is zero, and as the shaft starts spinning,
either the compressor or turbine first gets a pressure ratio greater than
1, which allows flow to begin. It can be seen that the mass flow rate is
actually slightly negative before the compressor finally activates. Then
as the pressure ratios further increase, the mass flow rate increases until
it reaches the steady condition.

!col! small=6 medium=6 large=6

!media thermal_hydraulics/modeling_guide/brayton_cycle/mfr_vs_time_open.png
       id=mfr_vs_time_open
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Mass flow rate transient for the open cycle.

!col-end!

!col! small=6 medium=6 large=6

!media thermal_hydraulics/modeling_guide/brayton_cycle/mfr_vs_time_closed.png
       id=mfr_vs_time_closed
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Mass flow rate transient for the closed cycle.

!col-end!

[p_ratio_vs_time_open] and [p_ratio_vs_time_closed] show the pressure ratio
transient for the open and closed cycles, respectively. Recall that this example
starts with everything at rest, i.e., there is initially no flow or shaft speed.
The motor ramps up the shaft speed to get the compressor working, but only
around $t = 80$ s does the pressure ratio of the compressor and turbine start
to exceed 1, which is the point at which flow can start to develop. In this case,
the pressure ratio in the compressor exceeds the turbine pressure ratio for the
majority of the transient, as the flow and shaft conditions seek to find an
equilibrium state, at which the pressure ratios of the compressor and turbine
are nearly equal.

!col! small=6 medium=6 large=6

!media thermal_hydraulics/modeling_guide/brayton_cycle/p_ratio_vs_time_open.png
       id=p_ratio_vs_time_open
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Pressure ratio transient for the open cycle.

!col-end!

!col! small=6 medium=6 large=6

!media thermal_hydraulics/modeling_guide/brayton_cycle/p_ratio_vs_time_closed.png
       id=p_ratio_vs_time_closed
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Pressure ratio transient for the closed cycle.

!col-end!

[power_vs_time_open] and [power_vs_time_closed] show the power
transient for the open and closed cycles, respectively. Several quantities are
included on these plots:

- $\dot{Q}_\text{in}$, the thermal power provided by the heat source,
- $\dot{Q}_\text{out}$, the thermal power provided by the heat sink (closed cycle only),
- $\dot{W}_\text{gen}$, the power taken by the generator (and thus becomes useful electrical power),
- $\dot{W}_\text{comp}$, the power consumed by the compressor, and
- $\dot{W}_\text{turb}$, the power provided by the turbine.

Not shown in these plots is the power consumed by the motor, as it dwarfs the
other powers shown for the period of the transient where it is most active.
In a practical setup, the motor power may be much less and just operates for
a much longer period during startup.

!col! small=6 medium=6 large=6

!media thermal_hydraulics/modeling_guide/brayton_cycle/power_vs_time_open.png
       id=power_vs_time_open
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Power transient for the open cycle.

!col-end!

!col! small=6 medium=6 large=6

!media thermal_hydraulics/modeling_guide/brayton_cycle/power_vs_time_closed.png
       id=power_vs_time_closed
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Power transient for the closed cycle.

!col-end!

Comparing the various plots between the open and closed cycles supports the
assertion that a closed Brayton cycle approximation can yield very similar
results to an open Brayton cycle.

!bibtex bibliography
