# PIDTransientControl

The `PIDTransientControl` object is designed to use the principle of Proportional Integral Derivative control to either:
- control a "Real" parameter from the input file rather than use the constant value specified in the input file.

- modify the value of a postprocessor, used in a PostprocessorDirichletBC for example, rather than use the computed/received value


This allows a simple 1D parametric optimization to make the output of a postprocessor match the `target` value.

The parameter $C$ is replaced at every time step $n$, at time $t$, by:
\begin{equation}
C_{n} = C_{n-1} + K_{integral} \int_0^{t} pp(s) - target \mathrm{d}s + K_{proportional} (pp(t) - target) + K_{derivative} \dfrac{pp(t) - target}{dt}
\end{equation}

with $pp(t)$ the value at time $t$ of the postprocessor that we are trying to match to the $target$ value and
$K_{integral/proportional/derivative}$ the coefficients for the integral error, current error, and backward derivative respectively.

!syntax parameters /Controls/PIDTransientControl

!syntax inputs /Controls/PIDTransientControl

!syntax children /Controls/PIDTransientControl
