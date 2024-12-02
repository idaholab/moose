# PIDChainControl

This [ChainControl](syntax/ChainControls/index.md) implements the classic PID controller, which takes as its input
a value $x$ and the set point for that value, $\bar{x}$. It produces an output
signal $y$, which should be used as the input for some controllable device that
impacts the measured quantity $x$. For example, in a thermal system, $x$ may
be the temperature of a fluid at some location, and $y$ may be the power sent
to the heaters in the system, with the goal to heat the fluid such that the
temperature at some location is $\bar{x}$.

"PID" stands for its three components:

- P: Proportional
- I: Integral
- D: Derivative

The output signal $y$ at time $t_n$ is computed as follows:

!equation
y_n = K_p e_n + K_i \sum\limits_{m=1}^n e_m \Delta t_m + K_d \frac{e_n - e_{n-1}}{\Delta t_n} \,,

where $\Delta t_n \equiv t_n - t_{n-1}$ is the time step size and
$e_n$ is the error:

!equation
e_n \equiv \bar{x}_n - x_n \,.

!alert warning title=Execute only once per time step
The implementation assumes that the control will only be executed once per
time step, so you should set [!param](/ChainControls/PIDChainControl/execute_on) accordingly,
such as `execute_on = 'INITIAL TIMESTEP_END'`, which is the default.

The inputs and outputs are retrieved and named as follows, respectively:

- $x$ is set with [!param](/ChainControls/PIDChainControl/input).
- $\bar{x}$ is set with [!param](/ChainControls/PIDChainControl/set_point).
- $y$ is declared with the name `<control_name>:value`, where `<control_name>`
  is the user-given name of the `PIDChainControl`.

!alert tip title=Tuning PID coefficients
If you are unsure on how to select the PID coefficients $K_p$, $K_i$, and $K_d$,
you may try the following strategy, which involves some trial and error. First,
set $K_i$ and $K_d$ to zero. Then, set $K_p$ to some arbitrary (positive) value.
Run a simulation with $x_0 \neq \bar{x}$ and examine the transient response of $x$
to the set point value $\bar{x}$. The goal should be to maximize $K_p$ without
causing an unstable response, where $x$ oscillates indefinitely around $\bar{x}$.
Some initial overshoot and subsequent diminishing oscillations are acceptable.
After this acceptable value of $K_p$ is found, you'll likely find that $x$ appears
to be nearly constant in time, but at the wrong value. At this point, you should
start increasing $K_i$ from zero until you get an acceptable response time to
get the initial response (largely driven by $K_p$) to bridge the gap to $\bar{x}$,
without introducing oscillatory behavior. Lastly, $K_d$ can be used to fine-tune
the response, but it is not necessary, and it risks some oscillatory behavior
if the inputs have any noise, so it should only be used for relatively smooth
inputs.

!alert note
To control a controllable value directly instead of using a [ChainControlData.md], use [PIDTransientControl.md].

!syntax parameters /ChainControls/PIDChainControl

!syntax inputs /ChainControls/PIDChainControl

!syntax children /ChainControls/PIDChainControl
