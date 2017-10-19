# Contact System
The contact system detects proximity and overlaps between surfaces in the mesh,
and enforces appropriate constraints on the mechanical and thermal behavior on
that interface. The mechanical contact system prevents penetration between
surfaces, and computes the contact forces on those surfaces required for the
non-penetration condition. The thermal contact system transfers heat between
surfaces.

## Mechanical Contact
Historically, there have been multiple approaches taken to enforce mechanical
contact in MOOSE. The Constraint contact system is the most robust of these
approaches, and is recommended for mechanical contact enforcement in both 2D and
3D simulations. In the future, Constraint will be the only option.

Currently, all mechanical contact enforcement algorithms in MOOSE are based on a
master/slave algorithm, where contact is enforced at the nodes on the slave
surface, which cannot penetrate faces on the master surface. As with all such
approaches, for the best results, the master surface should be the coarser of
the two surfaces.

The contact enforcement system relies on MOOSE's geometric search system to
provide the candidate set of faces that can interact with a slave node at a
given time. The set of candidate faces is controlled by the patch_size parameter
and the patch_update_strategy options in the Mesh block. The patch size must be
large enough to accommodate the sliding that occurs during a time step. It is
generally recommended that the patch_update_strategy=auto be used.

The formulation parameter specifies the technique used to enforce contact. The
DEFAULT option uses a kinematic enforcement algorithm that transfers the
internal forces at slave nodes to the corresponding master face, and forces the
slave node to be at a specific location on the master face using a penalty
parameter. The converged solution with this approach results no penetration
between the surfaces. The PENALTY algorithm employs a penalty approach, where
the penetration between the surfaces is penalized, and the converged solution
has a small penetration, which is inversely proportional to the penalty
parameter.

Regardless of the formulation used, the robustness of the mechanical contact
algorithm is affected by the penalty parameter. If the parameter is too small,
there will be excessive penetrations with the penalty formulation, and
convergence will suffer with the kinematic formulation. If the parameter is too
large, the solver may struggle due to poor conditioning.

## Thermal Contact
Thermal contact also uses the master/slave algorithm, and, as in mechanical
contact, the coarser meshed surface should be set as the master surface. In
thermal contact heat is transferred between the two surfaces.


!syntax objects /Contact

!syntax subsystems /Contact

!syntax actions /Contact
