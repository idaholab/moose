# FixedPointIterationAdaptiveDT

## Description

This time stepper adjusts the time step size according to a target number of fixed
point iterations $m_\text{target}$. This can only be used in apps having one or
more [MultiApps](syntax/MultiApps/index.md).

The time step size for step $n$, $\Delta t_n$, is computed as the product of the
old time step size, $\Delta t_{n-1}$, and a multiplier $\alpha$, which varies
based on the old time step number of fixed point iterations, $m_{n-1}$:

!equation
\Delta t_n = \alpha \Delta t_{n-1}

!equation
\alpha = \left\{\begin{array}{l l}
  1 & M_\text{min} \leq m_{n-1} \leq M_\text{max}\\
  \alpha_\text{increase} & m_{n-1} < M_\text{min}\\
  \alpha_\text{decrease} & m_{n-1} > M_\text{max}\\
\end{array}\right.

!equation
M_\text{min} = m_\text{target} - \delta m_\text{window}

!equation
M_\text{max} = m_\text{target} + \delta m_\text{window}

where

- $m_\text{target}$ is the target number of fixed point iterations, provided by
  [!param](/Executioner/TimeStepper/FixedPointIterationAdaptiveDT/target_iterations),
- $\delta m_\text{window}$ is the target window, provided by
  [!param](/Executioner/TimeStepper/FixedPointIterationAdaptiveDT/target_window),
- $\alpha_\text{increase}$ is the increase factor, provided by
  [!param](/Executioner/TimeStepper/FixedPointIterationAdaptiveDT/increase_factor), and
- $\alpha_\text{decrease}$ is the decrease factor, provided by
  [!param](/Executioner/TimeStepper/FixedPointIterationAdaptiveDT/decrease_factor).

The time step size of the first step is provided by
[!param](/Executioner/TimeStepper/FixedPointIterationAdaptiveDT/dt_initial).

!syntax parameters /Executioner/TimeSteppers/FixedPointIterationAdaptiveDT

!syntax inputs /Executioner/TimeSteppers/FixedPointIterationAdaptiveDT

!syntax children /Executioner/TimeSteppers/FixedPointIterationAdaptiveDT
