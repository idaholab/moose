# Conjugate Heat Transfer (CHT) Capability

This summarizes the design and details of the conjugate heat transfer capabilities
with the linear finite volume system through the [SIMPLE.md] executioner.

This capability is activated by specifying a boundary on the [!param](/Executioner/SIMPLE/cht_interfaces)
parameter. Other cht-related parameters can control the iteration between the solid and fluid
energy equations. Once the capability is activated it will check if the
used boundary conditions are compatible or not. In general, we introduced CHT versions of
common boundary conditions such as [LinearFVRobinCHTBC.md] and [LinearFVDirichletCHTBC.md]
that are dedicated for CHT applications.

For coupling purposes several new functors are created under the hood:

- +heat_flux_to_solid_*+ (where * is the interface boundary name),
- +heat_flux_to_fluid_*+ (where * is the interface boundary name),
- +interface_temperature_to_solid_*+ (where * is the interface boundary name),
- +interface_temperature_to_fluid_*+ (where * is the interface boundary name),

where the first two describe the heat flux from one domain to the other, while the
other two describe the interface temperature supplied to the named destination side.
The fluid boundary condition must use +interface_temperature_to_fluid_*+, and the
solid boundary condition must use +interface_temperature_to_solid_*+.

## Energy Conservation Equations

The energy conservation equations for fluid and solid domains are:

\begin{equation}
    \frac{\partial \rho h_f}{\partial t} + \nabla \cdot \left(\rho \mathbf{u} h_f \right) = \nabla \cdot \left(k_f \nabla T_f\right) + Q_f\,,
\end{equation}
\begin{equation}
    \frac{\partial \rho h_s}{\partial t} = \nabla \cdot \left(k_s \nabla T_s\right) + Q_s\,,
\end{equation}

Where $h_f = f(T_f)$ and $h_s = f(T_s)$ are the fluid and solid specific enthalpies, $k_f$ and $k_s$ are the thermal conductivities, and $Q_f$ and $Q_s$ are the external heat sources.

## Boundary Conditions

The coupling of the solid and fluid domains is done through boundary conditions that ensure:

1. **Interface Temperature Relation**
   \begin{equation}
       T_\mathrm{f,wall} - T_\mathrm{s,wall}
       = R_\mathrm{th}^{\prime\prime} q_\mathrm{f\rightarrow s}^{\prime\prime}
   \end{equation}

   where the area-normalized thermal resistance is $R_\mathrm{th}^{\prime\prime}$ and
   $q_\mathrm{f\rightarrow s}^{\prime\prime}$ is positive for heat flow from the fluid
   into the solid. Temperature continuity is recovered when
   $R_\mathrm{th}^{\prime\prime}=0$.

2. **Continuity of Conductive Flux at the Interface**
   \begin{equation}
       q_\mathrm{f,wall} = -q_\mathrm{s,wall}
   \end{equation}

## Interface Thermal Resistance

The [!param](/Executioner/SIMPLE/thermal_resistance) parameter specifies the
area-normalized thermal resistance in $\mathrm{m^2\,K/W}$. Its default value is zero.
It accepts either one functor that is used for every entry in
[!param](/Executioner/SIMPLE/cht_interfaces), or one functor per interface in the same
order as +cht_interfaces+. Every evaluated resistance must be nonnegative. A spatially
varying functor may be used, but it must be defined on at least one side of every face
on the corresponding CHT interface.

For each destination side, the handler applies the resistance drop to the boundary
temperature computed on the source side:

\begin{equation}
    T_\mathrm{to\,side} = T_\mathrm{other,wall}
    - R_\mathrm{th}^{\prime\prime} q_\mathrm{to\,side}^{\prime\prime}\,.
\end{equation}

The signed heat flux $q_\mathrm{to\,side}^{\prime\prime}$ follows the outward-flux
convention of the source side. Consequently, the two directional temperature functors
represent the values seen by their destination regions and naturally produce the two
sides of the temperature jump. The heat-flux continuity condition is unchanged.

## Coupling Methods

The methods currently recommended for CHT utilize [LinearFVDirichletCHTBC.md] and
[LinearFVRobinCHTBC.md] in the two different ways listed below. The Robin BC can also
emulate a Neumann BC by setting the [!param](/LinearFVBCs/LinearFVRobinCHTBC/h) parameter to 0.

- +Neumann-Dirichlet Coupling+

!algorithm
[!function!begin name=NeumannDirichletCoupling]
[!state text=Initialize $T_\mathrm{to\,fluid}^0$, $q_\mathrm{solid,wall}^0$]
[!while!begin condition=Convergence criteria not met]
[!state text=1. Solve fluid equation]
[!state text=2. Update heat flux from fluid to solid $q_\mathrm{s,wall}^n$]
[!state text=3. Solve solid equation]
[!state text=4. Update the temperature supplied to the fluid $T_\mathrm{to\,fluid}^n$]
[!while!end]
[!function!end]

- +Robin-Robin Coupling+

!algorithm
[!function!begin name=RobinRobinCoupling]
[!state text=Initialize $T_\mathrm{to\,fluid}^0$, $q_\mathrm{s\rightarrow f}^0 = 0$]
[!while!begin condition=Convergence criteria not met]
[!state text=1. Solve fluid equation with Robin boundary condition using $T_\mathrm{to\,fluid}^{n-1}$ and $q_\mathrm{s\rightarrow f}^{n-1}$]
[!state text=2. Update the temperature supplied to the solid $T_\mathrm{to\,solid}^n$]
[!state text=3. Update heat flux $q_\mathrm{f\rightarrow s}^n$]
[!state text=4. Solve solid equation with Robin boundary condition using $T_\mathrm{to\,solid}^{n}$ and $q_\mathrm{f\rightarrow s}^{n}$]
[!state text=5. Update the temperature supplied to the fluid $T_\mathrm{to\,fluid}^n$]
[!state text=6. Update heat flux $q_\mathrm{s\rightarrow f}^n$]
[!while!end]
[!function!end]


The Robin-Robin method introduces virtual heat transfer coefficients $h_f$ and $h_s$ to enhance stability and convergence.
