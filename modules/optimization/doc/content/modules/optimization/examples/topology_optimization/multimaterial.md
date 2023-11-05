# Ordered SIMP: Multimaterial

In addition to establish a maximum volume fraction and optimize the distribution
of material over the domain, one can optimize multiple materials within the same
domain and assign, additionally, individual cost to each of them
(see [!cite](tavakoli2014alternating)). This allows to
enforce another constraint: The total cost over the domain.

This approach uses the object `DensityUpdateTwoConstraints`, which allows multiple
material optimization to be applied to blocks individually. The materials in this
example have pseudo-densities of 0.4, 0.7, and 1.0; in addition to the void material
of 0.0.

Cost and Young's modulus are defined as interpolated quantities across the three
materials:

!listing examples/optimization/three_materials.i
         block=Materials id=bc_var_block_a
         caption=MBB Material interpolation for multimaterial optimization

The optimization process and the enforcement of volume and cost constraints
are driven by the `DensityUpdateTwoConstraints`, radial average, and
sensitivity filter user objects:

!listing examples/optimization/three_materials.i
         block=UserObjects id=bc_var_block_b
         caption=MBB User objects for multimaterial optimization


The final distribution of materials for the bridge structure (note that
symmetry is being used) looks as follows:

!media large_media/optimization/multimaterial.png style=width:75%



