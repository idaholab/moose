# ADSpecificImpulse1Phase

!syntax description /Postprocessors/ADSpecificImpulse1Phase

The flux on the boundary is first computed using the boundary user object for the boundary
of interest. Then we assume the process is isentropic and compute the inlet entropy $s_{in}$
from the inlet specific volume $v$ and specific internal energy $e$:

!equation
s_{in} = s(v_{in}, e_{in})

We know the outlet pressure, and we use the bisection method to compute the outlet
temperature considering the process is isentropic. From the outlet pressure and temperature, we can
compute the outlet enthalpy:

!equation
h_{out} = p(p_{out}, T_{out})

then compute the thrust from the outlet velocity and the mass flow rate $\dot{m}$ for the quadrature point as:

!equation
thrust_{qp} = |\sqrt(2 (h_{in} - h_{out})) \dot{m}|

The specific impulse $I_{sp}$ is finally returned from the boundary thrust as:

!equation
I_{sp} = \dfrac{thrust}{\dot{m} g}

!syntax parameters /Postprocessors/ADSpecificImpulse1Phase

!syntax inputs /Postprocessors/ADSpecificImpulse1Phase

!syntax children /Postprocessors/ADSpecificImpulse1Phase
