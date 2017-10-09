# Contact System

Finite element contact enforces constraints between surfaces in the mesh. Mechanical contact
prevents penetration and develops contact forces. Thermal contact transfers heat between the
surfaces. The Constraint contact system is recommended for mechanical contact in both
2D and 3D simulations.  The Constraint system requires the use of patch size in
the input file.

It is good practice to make the master surface the coarser of the two surfaces
in a contact problem.
Additionally, the robustness and accuracy of the mechanical contact algorithm is
strongly dependent on the penalty parameter. If the parameter is
too small, inaccurate solutions are more likely. If the
parameter is too large, the solver may struggle.

The `DEFAULT` option uses an enforcement algorithm that moves the internal forces at a slave
node to the master face. The distance between the slave node
and the master face is penalized.
The `PENALTY` algorithm is the traditional penalty enforcement technique.

!syntax objects /Contact

!syntax subsystems /Contact

!syntax actions /Contact
