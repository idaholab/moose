# Examples

These examples utilize the FunctionalExpansionTools module---frequently also using functionality from the HeatConduction module---to demonstrate a few use cases for MultiApp coupling in MOOSE. Picard iterations are used in all cases to converge the solutions.

All these examples are located in the `examples/` directory of the `functional_expansion_tools` module.

## Interface

### Cartesian

#### 2D_interface {2Dinterface}

Basic coupling between a master and sub app at a shared interface. The master app provides a flux term to the sub app, and the sub app returns both value and flux conditions.

#### 2D_interface_different_submesh

Demonstrates the mesh-agnosticity of FX methods by solving the exact same problem as **2D_interface**, but this time the mesh in the sub app is not identical to the master app. No extra configuration is required. The solution is comparable to that of **2D_interface**.

#### 2D_interface_no_material

Demonstrates a solution similar to **2D_interface**, but without any materials or dependence on the *HeatConduction* module for conduction kernels. Diffusion is used instead, straight from the main framework.

## Volumetric

### Cartesian

#### 1D_volumetric_Cartesian

Basic coupling between a master and sub app that share the same Cartesian physical space. The master app provides field values to the sub app, which then performs its calculation. The sub app's solution field is transferred back and coupled into the master app solution.

This approach couples the sub app solution to the mast app via the `FunctionSeriesToAux`+`CoupledForce` approach. This solution is slightly faster than a direct coupling using the `BodyForce`-based approach, but it can easily loose higher-order information with coarser meshes.

#### 2D_volumetric_Cartesian

2D version of **1D_volumetric_Cartesian**, with more divisions and a higher FX order in _y_

#### 3D_volumetric_Cartesian

3D version of **2D_volumetric_Cartesian**, with more divisions and a higher FX order in _z_

This is not a "light" simulation, and will probably take a few minutes to run.

#### 3D_volumetric_Cartesian_different_submesh

Demonstrates the mesh-agnosticity of FX methods by solving the exact same problem as **3D_volumetric_Cartesian**, but this time the mesh in the sub app is not identical to the master app. No extra configuration is required. The solution is comparable to that of **3D_volumetric_Cartesian**.

#### 3D_volumetric_Cartesian_direct

Demonstrates the `BodyForce`-based method of coupling the sub app solution into the master app solution. The solution is comparable to that of **3D_volumetric_Cartesian**. Further, the solution will be more accurate than the `CoupledForce`-based approach, but at a slight (< 10%) computational performance hit.

### Cylindrical

#### 3D_volumetric_cylindrical

Basic coupling between a master and sub app that share the same cylindrical physical space. The mesh used is from the file _cyl-tet.e_ found in _Example 6_ of the main MOOSE framework. The master app provides field values to the sub app, which then performs its calculation. The sub app's solution field is transferred back and coupled into the master app solution.

This approach couples the sub app solution to the mast app via the recommended `FunctionSeriesToAux`+`CoupledForce` approach.

#### 3D_volumetric_cylindrical_subapp_mesh_refine

Demonstrates the mesh-agnosticity of FX methods by solving the exact same problem as **3D_volumetric_cylindrical**, but this time mesh adaptivity is enabled in the sub app. This means that the sub app mesh is constantly changing throughout the simulation. The solution is comparable to that of **3D_volumetric_cylindrical**.

_Exodus_ output is enabled from the sub app so that the mesh changes can be visualized, ensuring that the sub app mesh changes throughout the solution.
