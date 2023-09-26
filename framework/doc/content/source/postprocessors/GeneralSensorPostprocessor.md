# GeneralSensorPostprocessor

## Syntax and Description

!syntax description /Postprocessors/GeneralSensorPostprocessor

`GeneralSensorPostprocessor` models very general sensor functions, when provided
with an input signal (the quantity of interest), along with parameters for evaluating
sensor noise with a Gaussian distribution (mean, standard deviation).
Additional options include sensor delay and drift function. The sensor equation
is as follows:

!equation
x(t) = s(t) + n(t) + d(t)

Where $x(t)$ is the signal recorded from the sensor, $s(t)$ is the measured signal,
$n(t)$ is the sensor noise, and $d(t)$ is the sensor drift function.

!syntax parameters /Postprocessors/GeneralSensorPostprocessor

!syntax inputs /Postprocessors/GeneralSensorPostprocessor

!syntax children /Postprocessors/GeneralSensorPostprocessor

!bibtex bibliography
