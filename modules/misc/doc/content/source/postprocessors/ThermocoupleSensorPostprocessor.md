## Syntax and Description

!syntax description /Postprocessors/GeneralSensorPostprocessor

`ThermocoupleSensorPostprocessor` object implements a thermocouple sensor that takes in the calculated input signal from moose and outputs a realistic and sensor-mediated output that accounts for efficiency, drift, delay and noise of the sensor in question. Different types of thermocouples are included. This is a child class of `GeneralSensorPostprocessor`.

From the `GeneralSensorPostprocessor`,
$$
 \tilde{u}(t) =  u(t) + f_{SN}(t)n(\sigma(t)) 
$$

$$
v(t) = \mu(t) + \eta(t) \left\{ K_p \tilde{u}(t-\tau(t)) + K_i \int_{0}^{t} \tilde{u}(t') R(t, t') dt' \right\} + \epsilon(\sigma(t))
$$

For a thermocouple,
$$
R(t, t') = \frac{e^{-(t-t')/\tau}}{\tau} ,
$$

$$
K_p=0, K_i=1,
$$ 

$$
v(t) =  \mu(t) + \eta(t)~\int_{0}^{t} \tilde{u}(t') e^{(t-t')/\tau}  dt'  + \epsilon(\sigma(t))
$$


!syntax parameters /Postprocessors/ThermocoupleSensorPostprocessor

!syntax inputs /Postprocessors/ThermocoupleSensorPostprocessor

!syntax children /Postprocessors/ThermocoupleSensorPostprocessor

!bibtex bibliography
*/