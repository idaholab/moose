# ExplicitDynamicsContactConstraint

!syntax description /Constraints/ExplicitDynamicsContactConstraint

This object implements node-face constraints for the enforcement of normal
mechanical contact in explicit dynamics. Surrogate balance-momentum equations are
solved at each node on the secondary surface using density and wave speed
material properties and the velocities of the two surfaces in contact.

For relevant equations, see [!citep](heinstein2000contact), in particular,
Equations (15), (21), (26) and (29).

!syntax parameters /Constraints/ExplicitDynamicsContactConstraint

!syntax inputs /Constraints/ExplicitDynamicsContactConstraint

!syntax children /Constraints/ExplicitDynamicsContactConstraint
