# ADShaftConnectedCompressor1PhaseUserObject

!syntax description /UserObjects/ADShaftConnectedCompressor1PhaseUserObject

A compressor is a type of volume junction, a [ADVolumeJunction1PhaseUserObject.md].
As such its base contribution to the residual and Jacobian is set by this class. An additional contribution to
the residual (and Jacobian) is described below:

The additional contribution to the residual of the energy equation is simply the power dissipated in the component:

!equation
R_e += (T_{friction} + T_{dissipation}) \omega

where $T_{friction}$ is the friction torque, $T_{dissipation}$ is the dissipation torque and $\omega$ is the rotation speed of
the compressor.

The additional contribution to the residual of the momentum equation in each direction is:

!equation
\vec{R} = \Delta p A \vec{d}_{out}

where $\Delta p$ is the pressure increase across the compressor, $A$ is the flow area and $\vec{d}_{out}$ the outlet direction.

The pressure increase in the compressor is:

!equation
\Delta p = p0_{in} * (Rp_{comp} - 1.0)

where $p0_{in}$ is inlet pressure, and $Rp_{comp}$ is the pressure ratio, or the inverse of the pressure ratio if treating the compressor
as a turbine.


The user object also provides APIs to retrieve the compressor's:

- isentropic torque
- dissipation torque
- friction torque
- pressure drop
- pressure ratio
- efficiency
- correct mass flow rate
- corrected speed

!alert note
This user object is created automatically by the [ShaftConnectedCompressor1Phase.md]
component, users do not need to add it to an input file.

!syntax parameters /UserObjects/ADShaftConnectedCompressor1PhaseUserObject

!syntax inputs /UserObjects/ADShaftConnectedCompressor1PhaseUserObject

!syntax children /UserObjects/ADShaftConnectedCompressor1PhaseUserObject
