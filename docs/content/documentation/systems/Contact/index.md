# Contact System

Finite element contact enforces constraints between surfaces in the mesh. Mechanical contact
prevents penetration and develops contact forces. Thermal contact transfers heat between the
surfaces. There are currently two systems to choose from for mechanical contact:
Dirac and Constraint. Constaint based contact is recommended for two-dimensional problems
and Dirac for three-dimensional problems. Constraint
contact is more robust but due to the patch
size requirement specified in the Mesh block constraint contact uses too much memory on 3D
problems. Depending upon the contact formalism chosen the solver options to be used change.

In LWR fuel analysis, the cladding surface is typically the master surface, and the fuel
surface is the slave surface. It is good practice to make the master surface the coarser
of the two. The robustness and accuracy of the mechanical contact algorithm is
strongly dependent on the penalty parameter. If the parameter is
too small, inaccurate solutions are more likely. If the
parameter is too large, the solver may struggle.
The DEFAULT option uses an enforcement algorithm that moves the internal forces at a slave
node to the master face. The distance between the slave node
and the master face is penalized.
The PENALTY algorithm is the traditional penalty enforcement technique.

!syntax objects /Contact

!syntax subsystems /Contact

!syntax actions /Contact
