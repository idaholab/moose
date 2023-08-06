# ADSimpleTurbine1PhaseUserObject

!syntax description /UserObjects/ADSimpleTurbine1PhaseUserObject

A turbine is a parallel channel junction, a [ADJunctionParallelChannels1PhaseUserObject.md], a type of volume junction,
a [ADVolumeJunction1PhaseUserObject.md]. As such its base contribution to the residual and Jacobian is set by these classes.

The additional residual for the momentum equation in each direction are:

!equation
\vec{R} = \Delta p A \vec{d}

where $\Delta p$ is the pressure drop across the turbine, as computed below, $A$ the flow area and $\vec{d}$ the direction of
the turbine.

!equation
\Delta p = p_{in} * (1 - (1 - \dfrac{_W_dot}{\rho uA h_{in})^{\dfrac{\gamma}{\gamma -1}})

where $p_{in}$ is the inlet pressure, $\rho uA$ is the inlet conserved momentum variable, $h_in$ is the inlet
enthalpy and $\gamma$ is the ratio of the specific isobaric and isochoric heat capacities of the fluid.

For the energy equation, the residual $R_e$ is simply the work $\dot{W}$ the turbine:

!equation
R_e = \dot{W}

!alert note
This user object is created automatically by the [SimpleTurbine1Phase.md]
component, users do not need to add it to an input file.

!syntax parameters /UserObjects/ADSimpleTurbine1PhaseUserObject

!syntax inputs /UserObjects/ADSimpleTurbine1PhaseUserObject

!syntax children /UserObjects/ADSimpleTurbine1PhaseUserObject
