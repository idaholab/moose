# Gap Conductance Constraint

!syntax description /Constraints/GapConductanceConstraint

The `GapConductanceConstraint` class is used to specify a heat flux across a gap
equivalent to $\frac{k}{l}\left(u_m - u_s\right)$ where $k$ is the gap
conductance, $l$ is the gap distance, $u_m$ is the temperature on the primary
side of the mortar interface, and $u_s$ is the temperature on the secondary
side. Constraint enforcement is done using lagrange multipliers.

!syntax parameters /Constraints/GapConductanceConstraint

!syntax inputs /Constraints/GapConductanceConstraint

!syntax children /Constraints/GapConductanceConstraint

!bibtex bibliography
