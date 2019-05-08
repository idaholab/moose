# GapConductanceConstraint

The `GapConductanceConstraint` class is used specify a heat flux across a gap
equivalent to $\frac{k}{l}\left(u_m - u_s\right)$ where $k$ is the gap
conductance, $l$ is the gap distance, $u_m$ is the temperature on the master
side of the mortar interface, and $u_s$ is the temperature on the slave
side. Constraint enforcement is done using lagrange multipliers.

!syntax description /Constraints/GapConductanceConstraint<RESIDUAL>

!syntax parameters /Constraints/GapConductanceConstraint<RESIDUAL>

!syntax inputs /Constraints/GapConductanceConstraint<RESIDUAL>

!syntax children /Constraints/GapConductanceConstraint<RESIDUAL>

!bibtex bibliography
