# GeneralSensorPostprocessor

!syntax description /Postprocessors/GeneralSensorPostprocessor

## Description

`GeneralSensorPostprocessor` object implements a general sensor that takes in the calculated input signal from moose and outputs a realistic and sensor-mediated output that accounts for efficiency, drift, delay and noise of the sensor in question. This acts as a base class for [ThermocoupleSensorPostprocessor.md] class. The user can also employ the base class to realistically model any general sensor different from a thermocouple or a neutron counter. 

Drift, delay, efficiency, signal to noise factor, uncertainty, standard deviation of noise and standard deviation of uncertainty can be either a constant or time-dependent functions. The noise and the uncertainty terms are determined by sampling from a Gaussian with a mean of zero and a user-supplied standard deviation. $K_{p}$ and $K_i$ are multipliers for the proportional and the integral terms, respectively. The following equation shows the equation used to implement the `GeneralSensorPostprocessor`. The nomenclature for the table is given in the Table.

\begin{equation}
 \tilde{u}(t) =  u(t) + f_{SN}(t)n(\sigma(t)) 
\end{equation}

\begin{equation}
v(t) = \mu(t) + \eta(t) \left\{ K_p \tilde{u}(t-\tau(t)) + K_i \int_{0}^{t} \tilde{u}(t') R(t, t') dt' \right\} + \epsilon(\sigma(t))
\end{equation}



| Symbol           | Meaning                                       |
|------------------|-----------------------------------------------|
| $t$              | Time                                          |
| $v(t)$           | The output signal of the sensor               |
| $u(t)$           | MOOSE-calculated unknown                      |
| $\tau(t)$        | Delay interval (may be time-dependent)        |
| $\mu(t)$         | Bias or drift term (may be time-dependent)    |
| $\eta(t)$        | Efficiency of sensor (may be time-dependent)  |
| $n(\sigma(t))$   | Probability density function for the noise    |
| $\sigma(t)$      | Standard deviation (may be time-dependent)    |
| $f_{SN}(t)$      | Signal-to-noise factor                         |
| $K_p$            | Multiplier for the proportional term           |
| $K_i$            | Multiplier for the integration term            |
| $\epsilon(\sigma(t))$ | Uncertainty                               |
| $R(t, t')$ | Function used to capture convolution over time for delay term |


!syntax parameters /Postprocessors/GeneralSensorPostprocessor

!syntax inputs /Postprocessors/GeneralSensorPostprocessor

!syntax children /Postprocessors/GeneralSensorPostprocessor

!bibtex bibliography