# 2D MBB Beam with a Convolution Filter and Adaptive Mesh Refinement

In this example we will go over the SIMP approach to topology optimization using adaptive mesh refinement. Only new material not covered in the previous
example will be covered here [2D Topology Optimization with Radial Average Filter](examples/2d_mbb.md), [2D Topology Optimization with PDE Filter and Boundary Penalty](examples/2d_mbb_pde.md).

The main addition for this example is showing an effective indicator for
adaptive mesh refinement, which is the `mat_den_nodal` variable below.. In the `AuxVariables` block there is a new variable.

```
  [mat_den_nodal]
    family = L2_LAGRANGE
    order = FIRST
    initial_condition = ${vol_frac}
    [AuxKernel]
      type = SelfAux
      execute_on = TIMESTEP_END
      variable = mat_den_nodal
      v = mat_den
    []
  []
```

The `mat_den_nodal` variable works better than using the `mat_den` variable for
the indicator. Using the `ValueJumpIndicator` on the `mat_den_nodal` variable
and refining the mesh where the indicator is greater than a small threshold
(0.1), will result in a mesh that is refined in the areas where the material is
not `0` or `1`. This helps to create a "sharp" interface between the void and
the solid elements, without having to refine the entire mesh.


!listing combined/examples/optimization/2d_mbb_pde_amr.i
         block=Adaptivity id=bc_var_block
         caption=MBB `Adaptivity` block
