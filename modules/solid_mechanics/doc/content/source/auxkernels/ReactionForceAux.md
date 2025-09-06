# ReactionForceAux

Save the reaction force corresponding to the variable DOFs into this AuxVariable.

!syntax parameters /AuxKernels/ReactionForceAux

# Usage

First declare a tagged vector to save the residual contribution, i.e.

!listing /test/tests/plane_stress/3D_finite_tension_pull.i block=Problem

Then, inform the SolidMechanics Physics about the declared vector tag. The Physics will automatically accumulate the residual contribution into the tagged vector. Importantly, this should only be done for the Kernels (which are set up by the SolidMechanics Physics), and not for other objects such as boundary conditions.

!listing /test/tests/plane_stress/3D_finite_tension_pull.i block=Physics

Next, use the ReactionForceAux to read the accumulated residual contribution into an AuxVariable, i.e.

!listing /test/tests/plane_stress/3D_finite_tension_pull.i block=AuxKernels

In this example, this would save the residual contribution in the $x$ direction into the AuxVariable named "react_x". Upon convergence of each time step, the residual contribution from the kernels will be close to zero (up to the tolerances you set) everywhere in the domain except where constraints or boundary conditions are applied. Examples of these constraints are Dirichlet/Neumann boundary conditions, pressure, applied torque, and contact, etc.. The final, accumulated residual contribution on the constrained degrees of freedom can then be interpreted as the distributed "reaction force".

Finally, if desired, the distributed reaction force can be summed up on a boundary to obtain the total reaction force. In the following example, the distributed reaction force is summed up on the "right" boundary where the displacement control is applied.

!listing /test/tests/plane_stress/3D_finite_tension_pull.i block=Postprocessors/react_x

# Example input files

!syntax inputs /AuxKernels/ReactionForceAux
