# ADShaftConnectedPump1PhaseUserObject

!syntax description /UserObjects/ADShaftConnectedPump1PhaseUserObject

A pump is a type of volume junction, a [ADVolumeJunction1PhaseUserObject.md].
As such its base contribution to the residual and Jacobian is set by this class. An additional contribution to
the residual (and Jacobian) is described below:


The additional contribution to the residual of the energy equation is simply the power of the pump:

!equation
R_e += (T_{hydraulic} + T_{dissipation}) \omega

where $T_{hydraulic}$ is the friction torque, and $\omega$ is the rotation speed of
the pump.

The additional contribution to the residual of the momentum equation is:

!equation
\vec{R} = \dfrac{\rho A}{A} \vec{g} H A_{ref} \vec{d}_{out}

where $\rho A$ is the conserved density, $A$ the local flow area, $A_{ref}$ is the reference flow area, $\vec{g}$ is the gravity vector,
$H$ the pump head, and $\vec{d}_{out}$ the outlet direction.

The pump head is computed as:

!equation
H_{pump} =  ( (\dfrac{\omega}{\omega_{rated}})^2 + (\dfrac{\dfrac{\rho u A A}{\rho A}}{Q_{rated}})^2 ) H_{function} H_{rated}

where $\omega_{rated}$ is the rated rotation speed of the pump, $\rho$ the fluid density, $u$ the fluid velocity, $A$ the local area,
$Q_{rated}$ is the rated flow rate, $H_{function}$ an adimensional pump head function and $H_{rated}$ is the rated head of the pump.

The user object also provides APIs to retrieve the pump's:

- hydraulic torque
- friction torque
- pump head

!alert note
This user object is created automatically by the [ShaftConnectedPump1Phase.md]
component, users do not need to add it to an input file.

!syntax parameters /UserObjects/ADShaftConnectedPump1PhaseUserObject

!syntax inputs /UserObjects/ADShaftConnectedPump1PhaseUserObject

!syntax children /UserObjects/ADShaftConnectedPump1PhaseUserObject
