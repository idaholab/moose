# Simply supported square plate under a uniform pressure applied as a
# distributed load, DSG shell elements of every member of the family.
#
# The plate of plate_bending.i (one quarter of 50 m x 50 m x 1 m, E = 1e9,
# nu = 0.3) carries the pressure q = 10.816 N/m^2 (positive against the Z axis, downward) through the distributed
# load kernel instead of nodal forces, on a 10 x 10 mesh of the quarter plate.
# The supported edges are hard simple supports: the transverse displacement
# and the rotation about the edge normal are zero, so the solution has no
# boundary layer and converges with the mesh.
#
# The thin plate center deflection is 2.998536e-03 m (Navier series). The
# shear deformation of the plate (t / a = 1 / 50) adds 0.22 percent, so the
# converged deflection of the shear-deformable plate is 3.0050e-03 m, which
# the six-node and eight-node members reach to five digits on this mesh.
# The element type and the variable order are set from the command line of
# the test specification for the three-node, six-node and eight-node members.

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin = 0.0
    xmax = 25.0
    ymin = 0.0
    ymax = 25.0
    elem_type = QUAD4
  []
[]

[Variables]
  [disp_x]
    order = FIRST
    family = LAGRANGE
  []
  [disp_y]
    order = FIRST
    family = LAGRANGE
  []
  [disp_z]
    order = FIRST
    family = LAGRANGE
  []
  [rot_x]
    order = FIRST
    family = LAGRANGE
  []
  [rot_y]
    order = FIRST
    family = LAGRANGE
  []
  [rot_z]
    order = FIRST
    family = LAGRANGE
  []
[]

[BCs]
  [symm_left_rot]
    type = DirichletBC
    variable = rot_y
    boundary = left
    value = 0.0
  []
  [symm_bottom_rot]
    type = DirichletBC
    variable = rot_x
    boundary = bottom
    value = 0.0
  []
  [simply_support_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'right top bottom left'
    value = 0.0
  []
  [simply_support_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'right top bottom left'
    value = 0.0
  []
  [simply_support_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'right top'
    value = 0.0
  []
  [hard_support_right]
    type = DirichletBC
    variable = rot_x
    boundary = right
    value = 0.0
  []
  [hard_support_top]
    type = DirichletBC
    variable = rot_y
    boundary = top
    value = 0.0
  []
[]

[Kernels]
  [solid_disp_x]
    type = StressDivergenceDSGShell
    component = 0
    variable = disp_x
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_disp_y]
    type = StressDivergenceDSGShell
    component = 1
    variable = disp_y
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_disp_z]
    type = StressDivergenceDSGShell
    component = 2
    variable = disp_z
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_rot_x]
    type = StressDivergenceDSGShell
    component = 3
    variable = rot_x
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_rot_y]
    type = StressDivergenceDSGShell
    component = 4
    variable = rot_y
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_rot_z]
    type = StressDivergenceDSGShell
    component = 5
    variable = rot_z
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [pressure]
    type = DistributedLoadDSGShell
    variable = disp_z
    displacements = 'disp_x disp_y disp_z'
    function = '10.816'
  []
[]

[Materials]
  [strain]
    type = ComputeDSGShellStrain
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [section]
    type = ComputeDSGShellSectionStiffness
    youngs_modulus = 1e9
    poissons_ratio = 0.3
    thickness = 1.0
  []
  [resultants]
    type = ComputeDSGShellResultants
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  dt = 1.0
  dtmin = 1.0
  end_time = 1.0
[]

[Postprocessors]
  [disp_z_center]
    type = PointValue
    point = '0.0 0.0 0.0'
    variable = disp_z
  []
[]

[Outputs]
  csv = true
[]
