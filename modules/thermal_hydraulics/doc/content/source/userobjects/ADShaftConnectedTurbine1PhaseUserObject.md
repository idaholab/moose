# ADShaftConnectedTurbine1PhaseUserObject

!syntax description /UserObjects/ADShaftConnectedTurbine1PhaseUserObject

A turbine is a volume junction, derived from 
[ADVolumeJunction1PhaseUserObject.md]. As such its base contribution to the residual and Jacobian is set by this parent class.

The additional residual for the momentum equation in each direction are:

!equation
\vec{R} = \Delta p A \vec{d}_{out}

where $\Delta p$ is the pressure drop across the turbine, as computed below, $A$ the flow area and $\vec{d}$ the direction of
the turbine outlet.

!equation
\Delta p =  (\rho V / volume) g_H;

where $\rho V$ is a variable, $volume$ the volume of the junction and $g_H$ is defined below:

!equation
g_H = head D_{wheel}^2 \omega^2

where $head$ is the head coefficient, $D_{wheel}$ is the diameter of the turbine, and $\omega$ is the inlet rotation speed.

For the energy equation, the additional residual $R_e$ is simply the power of the turbine $\dot{W}$.

!equation
\dot{W} = T \omega

with $T$ the torque of the turbine and $\omega$ its rotation speed.

The user object also provides APIs to retrieve the turbine's:

- driving torque
- friction torque
- flow coefficient
- pressure drop
- power

!alert note
This user object is created automatically by the [ShaftConnectedTurbine1Phase.md]
component, users do not need to add it to an input file.

!syntax parameters /UserObjects/ADShaftConnectedTurbine1PhaseUserObject

!syntax inputs /UserObjects/ADShaftConnectedTurbine1PhaseUserObject

!syntax children /UserObjects/ADShaftConnectedTurbine1PhaseUserObject
