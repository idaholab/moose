# PIDAdaptiveDT

## Description

This time stepper adjusts the time step size to control the estimated local time discretization error with a PID (proportional-integral-derivative) control logic based on a method published in [the paper](https://doi.org/10.1016/j.anucene.2023.109880) by Mustafa K. Jaradat and Won Sik Yang.

The time step size of the first step is provided by [!param](/Executioner/TimeStepper/PIDAdaptiveDT/dt_initial).
The local time discretization error can be estimated with two options with [!param](/Executioner/TimeStepper/PIDAdaptiveDT/error_esitmation_type).
The first is a second-order scheme with

!equation
\hat{R}_{n+1} = \frac{1}{2 \epsilon \left\| u_n \right\|_2} \left\| u_n - (1 + \frac{\Delta t_n}{\Delta t_{n-1}})u_{n-1} + \frac{\Delta t_n}{\Delta t_{n-1}}u_{n-2} \right\|_2,

where $u_n$ is the solution at time step $n$. $\Delta t_n$ is the time step size at time step $n$. $\epsilon$ is the user-prescribed tolerance with [!param](/Executioner/TimeStepper/PIDAdaptiveDT/local_error_tolerance).

!alert note
When [!param](/Executioner/TimeStepper/PIDAdaptiveDT/local_error_tolerance) is not provided, the code will use the estimated error at the first time step with the initial time step size as the local error tolerance.
When users use the first several time steps to verify whether a null transient holds before starting the real transient, the local error tolerance will be estimated after the real transient starts.

The solution $u$ is composed of all the field variables in the primal nonlinear system by default.
However, users can provide a list of field variables with [!param](/Executioner/TimeStepper/PIDAdaptiveDT/variables) for the evaluation.
The variables can be auxiliary variables.

The second option is a first-order scheme with

!equation
\hat{R}_{n+1} = \frac{\left\| u_n - u_{n-1} \right\|_2}{\epsilon \left\| u_n \right\|_2}.

The time step size at time step $n+1$ is evaluated with $\hat{R}$ as

!equation
\Delta t_{n+1} = \hat{R}^{-k_\text{P}}_{n+1} \hat{R}^{-k_\text{I} + k_\text{D}}_{n} \hat{R}^{-k_\text{D}}_{n-1} \Delta t_{n},

where $k_\text{P}, k_\text{I}, k_\text{D}$ are the proportional, integral and derivative gains that can be specified by [!param](/Executioner/TimeStepper/PIDAdaptiveDT/proportional_gain), [!param](/Executioner/TimeStepper/PIDAdaptiveDT/integral_gain) and [!param](/Executioner/TimeStepper/PIDAdaptiveDT/derivative_gain) respectively.
Because we typically do not expect suddent changes in the solution during a transient, setting [!param](/Executioner/TimeStepper/PIDAdaptiveDT/proportional_gain) is often sufficient, which is the reason why the other gains are defaulted to zero.

!alert note
Large value of [!param](/Executioner/TimeStepper/PIDAdaptiveDT/proportional_gain) can lead to oscillation of the time step size.
Because the parameter is problem dependent, users can set a relative large value and gradually reduce it until the oscillation disappears.
The proportional gain is often correlated with [!param](/Executioner/TimeStepper/PIDAdaptiveDT/local_error_tolerance), the lower the tolerance is, the smaller the proportional gain is needed.

!alert note
The formulation is not the same as the one in [the paper](https://doi.org/10.1016/j.anucene.2023.109880), but the three gains can be linearly transformed to the three gains here, and vice versa.

This time stepper also provides two parameters [!param](/Executioner/TimeStepper/PIDAdaptiveDT/max_increase_factor) and [!param](/Executioner/TimeStepper/PIDAdaptiveDT/min_decrease_factor) to avoid sharp changes in the time step size by limitting the ratio of the current time step size and the previous time step size within a specified range.
It also provides a parameter [!param](/Executioner/TimeStepper/PIDAdaptiveDT/timestep_limiting_postprocessor) that can be used to limit the time step size based on a postprocessor value.

!syntax parameters /Executioner/TimeSteppers/PIDAdaptiveDT

!syntax inputs /Executioner/TimeSteppers/PIDAdaptiveDT

!syntax children /Executioner/TimeSteppers/PIDAdaptiveDT
