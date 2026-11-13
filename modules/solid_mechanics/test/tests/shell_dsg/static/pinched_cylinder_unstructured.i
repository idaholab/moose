# Pinched cylinder on an unstructured mesh, DSG shell elements (the
# pinched_cylinder_symm_unstructured case of the module's shell battery).
#
# One eighth of the cylinder of pinched_cylinder.i (radius 1, half-length 1, thickness
# 0.01, E = 1e6, nu = 0.3) on an unstructured quadrilateral mesh: the rigid diaphragm is
# the circle z = 0 (boundary front), the loaded symmetry plane the circle z = 1 (back),
# and the two symmetry planes x = 0 (left) and y = 0 (right). The load P = 10 N is
# applied as 2.5 N on the eighth, at the node (1, 0, 1).
#
# The series solution gives 0.16424 m under the load; the structured 10 x 10 mesh of
# pinched_cylinder.i gives 0.1337 m, and this mesh a value of the same order.

[Mesh]
  [cylinder]
    type = FileMeshGenerator
    file = pinched_cylinder_unstructured.msh
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
  # Symmetry plane x = 0
  [symmetry_x_disp]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [symmetry_x_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = left
    value = 0.0
  []
  [symmetry_x_rot_z]
    type = DirichletBC
    variable = rot_z
    boundary = left
    value = 0.0
  []
  # Symmetry plane y = 0
  [symmetry_y_disp]
    type = DirichletBC
    variable = disp_y
    boundary = right
    value = 0.0
  []
  [symmetry_y_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = right
    value = 0.0
  []
  [symmetry_y_rot_z]
    type = DirichletBC
    variable = rot_z
    boundary = right
    value = 0.0
  []
  # Symmetry plane z = 1
  [symmetry_z_disp]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  []
  [symmetry_z_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = back
    value = 0.0
  []
  [symmetry_z_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = back
    value = 0.0
  []
  # Rigid diaphragm z = 0
  [diaphragm_x]
    type = DirichletBC
    variable = disp_x
    boundary = front
    value = 0.0
  []
  [diaphragm_y]
    type = DirichletBC
    variable = disp_y
    boundary = front
    value = 0.0
  []
[]

[DiracKernels]
  [point]
    type = ConstantPointSource
    variable = disp_x
    point = '1 0 1'
    value = -2.5
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
[]

[Materials]
  [strain]
    type = ComputeDSGShellStrain
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [section]
    type = ComputeDSGShellSectionStiffness
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    thickness = 0.01
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
  [disp_x_under_load]
    type = PointValue
    point = '1 0 1'
    variable = disp_x
  []
  [disp_y_opposite]
    type = PointValue
    point = '0 1 1'
    variable = disp_y
  []
[]

[Outputs]
  csv = true
[]
