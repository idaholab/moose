# Pinched cylinder with rigid end diaphragms, DSG shell elements.
#
# Figure 10 and Table 6 of Dvorkin and Bathe, Engineering Computations, 1984.
# A cylinder of radius 1 m and length 2 m (along Z), closed by rigid
# diaphragms at z = 0 and z = 2, is pinched at mid-length by two opposite
# point loads of 10 N at (1, 0, 1) and (-1, 0, 1). E = 1e6 Pa, nu = 0.3 and
# t = 0.01 m. One eighth of the cylinder is modeled: the quarter of the
# circumference from (1, 0, z) to (0, 1, z) between the symmetry plane z = 1
# and the diaphragm z = 2, so the load on the model is P / 4 = 2.5 N.
#
# The mesh is a structured 10 x 10 grid of the angle and the length, mapped
# onto the cylinder. Its boundaries keep the names of the flat grid: left is
# the line (1, 0, z) in the symmetry plane y = 0, right the line (0, 1, z) in
# the symmetry plane x = 0, bottom the symmetry plane z = 1 and top the
# diaphragm z = 2.
#
# The series solution for the radial displacement under the load is
# w = 164.24 P / (E t) = 0.16424 m. The ratio of the finite element result to
# this value is the figure of merit of the benchmark.

[Mesh]
  [grid]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin = 0.0
    xmax = 1.5707963267948966
    ymin = 1.0
    ymax = 2.0
    elem_type = QUAD4
  []
  [cylinder]
    type = ParsedNodeTransformGenerator
    input = grid
    x_function = 'cos(x)'
    y_function = 'sin(x)'
    z_function = 'y'
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
  # Symmetry plane y = 0
  [symmetry_y_disp]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0.0
  []
  [symmetry_y_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = left
    value = 0.0
  []
  [symmetry_y_rot_z]
    type = DirichletBC
    variable = rot_z
    boundary = left
    value = 0.0
  []
  # Symmetry plane x = 0
  [symmetry_x_disp]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0.0
  []
  [symmetry_x_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = right
    value = 0.0
  []
  [symmetry_x_rot_z]
    type = DirichletBC
    variable = rot_z
    boundary = right
    value = 0.0
  []
  # Symmetry plane z = 1
  [symmetry_z_disp]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  []
  [symmetry_z_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = bottom
    value = 0.0
  []
  [symmetry_z_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = bottom
    value = 0.0
  []
  # Rigid diaphragm z = 2
  [diaphragm_x]
    type = DirichletBC
    variable = disp_x
    boundary = top
    value = 0.0
  []
  [diaphragm_y]
    type = DirichletBC
    variable = disp_y
    boundary = top
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
