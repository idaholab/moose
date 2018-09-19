# Contact Module

The interaction of moving bodies is a common occurrence in our world, and therefore modeling such problems is essential to accurately represent the mechanical behavior of the physic world. However, finite element methods do not have an inherent means of modeling contact. Therefore, specific contact algorithms are required. These algorithms enforce constraints between surfaces in the mesh, to prevent penetration and develop contact forces. The MOOSE contact module provides the necessary tools for modeling mechanical contact.

[](---)

## Theory

Mechanical contact between fuel pellets and the inside surface of the cladding is based on three requirements.

\begin{equation*}
g \le 0,
\end{equation*}
\begin{equation*}
t_N \ge 0,
\end{equation*}
\begin{equation*}
t_N g = 0.
\end{equation*}


That is, the penetration distance (typically referred to as the gap $g$ in the contact literature) of one of the body into another must not be positive; the contact force $t_N$ opposing penetration must be positive in the normal direction; and either the penetration distance or the contact force must be zero at all times.

In the MOOSE Contact Module, these contact constraints are enforced through the use of node/face constraints. Specifically, the nodes of the fuel pellets are prevented from penetrating cladding faces. This is accomplished in a manner similar to that detailed by Heinstein and Laursen. First, a geometric search determines which master nodes have penetrated slave faces. For those nodes, the internal force computed by the divergence of stress is moved to the appropriate slave face at the point of contact. Those forces are distributed to slave nodes by employing the finite element shape functions. Additionally, the master nodes are constrained to remain on the pellet faces, preventing penetration. The module currently supports frictionless and tied contact. Friction is an important capability, and preliminary support for frictional contact is available.

Finite element contact is notoriously difficult to make efficient and robust in three dimensions. That being the case, effort is underway to improve contact algorithm.

[](---)

## Procedure for using mechanical contact

In the contact module there are currently two sytems to choose from mechanical contact : Dirac and Constraint. Constraint based contact is recommended for two-dimensional problems and Dirac for three-dimensional problems. Constraint contact is more robust but due to the patch size requirement specified in the `Mesh` block, Constraint contact uses too much memory on 3D problems. Depending upon the contact formalism chosen, the solver options to be used change. The details of the solver parameters recommended for Dirac and Constraint contact formalisms are provided below.

The contact block in the MOOSE input file looks like this :

```puppet
[Contact]
  [./contact]
    disp_x = <variable>
    disp_y = <variable>
    disp_z = <variable>
    formulation = <string> (DEFAULT)
    friction_coefficient = <real> (0)
    master = <string>
    model = <string> (frictionless)
    normal_smoothing_distance = <real>
    normal_smoothing_method = <string> (edge_based)
    order = <string> (FIRST)
    penalty = <real> (1e8)
    normalize_penalty = <bool> (false)
    slave = <string>
    system = <string> (Dirac)
    tangential_tolerance = <real>
    tension_release = <real> (0)
  [../]
[]
```

The parameters descriptions are :

- `disp_x` (+Required+) Variable name for displacement variable in x direction. Tipically `disp_x`.
- `disp_y` Variable name for displacement variable in y direction. Typically `disp_y`.
- `disp_z` Variable name for displacement variable in z direction. Typically `disp_z`
- `formulation` Select either `DEFAULT`, `KINEMATIC`, or `PENALTY`. `DEFAULT` is `KINEMATIC`.
- `friction_coefficient` The friction coefficient.
- `master` (+Required+) The boundary ID for the master surface.
- `model` Select either `frictionless`, `glued`, or `coulomb`.
- `normal_smoothing_distance` Distance from face edge in parametric coordinates over which to smooth the contact normal. $0.1$ is a reasonable value.
- `normal_smoothing_method` Select either `edge_based` or `nodal_normal_based`. If `nodal_normal_based`, must also have a `NodalNormals` block.
- `order` The order of the variable. Typical values are `FIRST` and `SECOND`.
- `penalty` The penalty stiffness value to be used in the constraint.
- `normalize_penalty` Whether to normalize the penalty stiffness by the nodal area of the slave node.
- `slave` (+Required+) The boundary ID for the slave surface.
- `system` The system to use for constraint enforcement. Options are Dirac `DiracKernel` or `Constraint`. The default is `Dirac`.
- `tangential_tolerance` Tangential distance to extend edges of contact surfaces.
- `tension_release` Tension release threshold. A node will not be released if its tensile load is below this value. If negative, no tension release will occur.


It is good practice to make the surface with the coarser mesh to be the master surface.

The robustness and accuracy of the mechanical contact algorithm is strongly dependent on the penalty parameter. If the parameter is too small, inaccurate solutions are more likely. If the parameter is too large, the solver may struggle.

The `DEFAULT` option uses an enforcement algorithmn that moves th internal forces at a slave node to th master face. The distance between the slave node and the master face is penalized, The `PENALTY` algorithm is the traditional penalty enforcement technique.

[](---)

## Petsc options for contact

The recommended PETSc options for use with Constraint based contact are shown below :

```puppet
[Executioner]
  ...
  petsc_options_iname = '-pc_type -sub_pc_type -pc_asm_overlap
                        -ksp_gmres_restart'
  petsc_options_value = 'asm lu 20 101'
  ...
[../]
```

The recommended PETSc options for use with Dirac based contact are given below:

```puppet
[Executioner]
  ...
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type
                        -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201 hypre boomeramg 4'
  ...
[../]
```

## Objects, Actions, and Syntax

!syntax complete group=ContactApp level=3
