# HeatStructureMaterials System

The `HeatStructureMaterials` system is used to define the material properties
of heat structures, namely the following properties, which are assumed to be
functions of temperature, $T$:

- Density: $\rho(T)$,
- Specific heat capacity: $c_p(T)$, and
- Thermal conductivity: $k(T)$.

These properties are relevant to the heat conduction equations that are active
on heat structure domains.

!syntax list /HeatStructureMaterials objects=True actions=False subsystems=False

!syntax list /HeatStructureMaterials objects=False actions=False subsystems=True

!syntax list /HeatStructureMaterials objects=False actions=True subsystems=False
