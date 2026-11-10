# Plate with force and moment loads, one DSG shell element.
#
# Shell verification problem 1.3.13 of the Abaqus verification manual. A plate
# of 40 m x 20 m x 1 m with E = 1000 Pa and nu = 0.3 is modeled with a single
# four-node element under the following boundary and loading conditions:
#   disp_z = 0 at the vertices A (0, 0), B (40, 0) and D (0, 20);
#   disp_x and disp_y zero at all four vertices;
#   F_z = -2.0 N at the vertex C (40, 20);
#   M_x = 20.0 N m at the vertices A and B (bottom edge) and -20.0 N m at
#   C and D (top edge);
#   M_y = 10.0 N m at B and C (right edge) and -10.0 N m at A and D (left
#   edge).
# The deflection at C is -12.54 m with S4 elements in Abaqus.

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
    xmin = 0.0
    xmax = 40.0
    ymin = 0.0
    ymax = 20.0
  []
  [c_node]
    type = ExtraNodesetGenerator
    input = gmg
    new_boundary = 100
    coord = '40.0 20.0'
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [rot_x]
  []
  [rot_y]
  []
  [rot_z]
  []
[]

[BCs]
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
    boundary = 'bottom left'
    value = 0.0
  []
[]

[NodalKernels]
  [force_C]
    type = ConstantRate
    variable = disp_z
    boundary = 100
    rate = -2.0
  []
  [Mx_AB]
    type = ConstantRate
    variable = rot_x
    boundary = bottom
    rate = 20.0
  []
  [Mx_CD]
    type = ConstantRate
    variable = rot_x
    boundary = top
    rate = -20.0
  []
  [My_BC]
    type = ConstantRate
    variable = rot_y
    boundary = right
    rate = 10.0
  []
  [My_AD]
    type = ConstantRate
    variable = rot_y
    boundary = left
    rate = -10.0
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
    youngs_modulus = 1000.0
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
  [disp_z_C]
    type = PointValue
    point = '40.0 20.0 0.0'
    variable = disp_z
  []
[]

[Outputs]
  csv = true
[]
