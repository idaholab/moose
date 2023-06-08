# DelayControl

This control takes one input and produces one output value.  The output value is the input value
delayed in time by a time period, $\tau$.  The constant cannot be changed during the simulation.

The formula used for computing the output value is:

\begin{equation}
\mathrm{output}(t) = \mathrm{input}(t - \tau)
\end{equation}

!syntax parameters /ControlLogic/DelayControl

!syntax inputs /ControlLogic/DelayControl

!syntax children /ControlLogic/DelayControl
