# Conjugate Heat Transfer (CHT) Capability

This summarizes the design and details of the conjugate heat transfer capabilities
with the linear finite volume system through the [SIMPLE.md] execitoner.

This capability is activate by specifying a boundary on the [!param](/Executioner/SIMPLE/bcht_interfaces)
parameter. Other cht-related parameters can control the iteration between the solid and fluid
energy equations. Once the capability is activated it will check if the
used boundary conditions are compatible or not. In general, we introduced CHT versions of
common boundary conditions such as [LinearFVRobinCHTBC.md] and [LinearFVDirichletCHTBC.md]
that are dedicated for CHT applications.

For coupling purposes several new functors are created under the hood:

- +heat_flux_to_solid_*+ (where * is the interface boudary name),
- +heat_flux_to_fluid_*+ (where * is the interface boudary name),
- +interface_temperature_solid_*+ (where * is the interface boudary name),
- +interface_temperature_fluid_*+ (where * is the interface boudary name),

where the first two describe the heat flux from one domain to the other, while the
other express the interface temperatures from both sides.

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

1. **Continuity of Interface Temperature**
   \begin{equation}
       T_\mathrm{f,wall} = T_\mathrm{s,wall}
   \end{equation}

2. **Continuity of Conductive Flux at the Interface**
   \begin{equation}
       q_\mathrm{f,wall} = -q_\mathrm{s,wall}
   \end{equation}

## Coupling Methods

The currenty recommended methods utilize [LinearFVDirichletCHTBC.md] and
[LinearFVRobinCHTBC.md] in the two different ways listed below. The Robin BC can also
emulate a neuamnn BC.

- +Neumann-Dirichlet Coupling+

!algorithm
[!function!begin name=NeumannDirichletCoupling]
[!state text=Initialize $T_\mathrm{fluid,wall}^0$, $q_\mathrm{solid,wall}^0$]
[!while!begin condition=Convergence criteria not met]
[!state text=1. Solve fluid equation]
[!state text=2. Update heat flux from fluid to solid $q_\mathrm{s,wall}^n$]
[!state text=3. Solve solid equation]
[!state text=4. Update boundary temperature $T_\mathrm{f,wall}^n$]
[!while!end]
[!function!end]

- +Robin-Robin Coupling+

!algorithm
[!function!begin name=RobinRobinCoupling]
[!state text=Initialize $T_\mathrm{fluid,wall}^0$, $q_\mathrm{s\rightarrow f}^0 = 0$]
[!while!begin condition=Convergence criteria not met]
[!state text=1. Solve fluid equation with Robin boundary condition using $T_\mathrm{s,wall}^{n-1}$ and $q_\mathrm{s\rightarrow f}^{n-1}$]
[!state text=2. Update wall temperature $T_\mathrm{f,wall}^n$]
[!state text=3. Update heat flux $q_\mathrm{f\rightarrow s}^n$]
[!state text=4. Solve solid equation with Robin boundary condition using $T_\mathrm{f,wall}^{n}$ and $q_\mathrm{f\rightarrow s}^{n}$]
[!state text=5. Update wall temperature $T_\mathrm{s,wall}^n$]
[!state text=6. Update heat flux $q_\mathrm{s\rightarrow f}^n$]
[!while!end]
[!function!end]


The Robin-Robin method introduces virtual heat transfer coefficients $h_f$ and $h_s$ to enhance stability and convergence.
