# Shell element verification test from Abaqus verification manual 1.3.13

# A 40 m x 20 m x 1 m plate that has E = 1000 Pa and Poisson's ratio = 0.3
# is subjected to the following boundary/loading conditions. A single shell
# element is used to model the plate.

# disp_z = 0 at vertices A (0, 0), B (40, 0) and D (20, 0).
# disp_x and disp_y are zero at all four vertices.

# F_z = -2.0 N at vertex C (40, 20).
# M_x = 20.0 Nm at vertices A and B (bottom boundary)
# M_x = -20.0 Nm at vertices C and D (top boundary)
# M_y = 10.0 Nm at vertices B and C (right boundary)
# M_y = -10.0 Nm at vertices A and D (left boundary)

# The disp_z at vertex C is -12.54 m using S4 elements in Abaqus.
# The solution obtained using Moose is -12.519 m with a relative error
# of 0.16%.

[Mesh]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
    xmin = 0.0
    xmax = 40.0
    ymin = 0.0
    ymax = 20.0
  [../]

  [./c_node]
    type = ExtraNodesetGenerator
    input = gmg
    new_boundary = 100
    coord = '40.0 20.0'
  [../]
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_y]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[BCs]
  [./simply_support_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'right top bottom left'
    value = 0.0
  [../]
  [./simply_support_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'right top bottom left'
    value = 0.0
  [../]
  [./simply_support_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'bottom left'
    value = 0.0
  [../]
[]

[NodalKernels]
  [./force_C]
    type = ConstantRate
    variable = disp_z
    boundary = 100
    rate = -2.0
  [../]
  [./Mx_AB]
    type = ConstantRate
    variable = rot_x
    boundary = bottom
    rate = 20.0
  [../]
  [./Mx_CD]
    type = ConstantRate
    variable = rot_x
    boundary = top
    rate = -20.0
  [../]
  [./My_BC]
    type = ConstantRate
    variable = rot_y
    boundary = right
    rate = 10.0
  [../]
  [./My_AD]
    type = ConstantRate
    variable = rot_y
    boundary = left
    rate = -10.0
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  line_search = 'none'
  #nl_max_its = 2
  nl_rel_tol = 1e-10
  nl_abs_tol = 6e-6

  dt = 1.0
  dtmin = 1.0
  end_time = 3
[]

[Kernels]
  [./solid_disp_x]
    type = ADStressDivergenceShell
    block = '0'
    component = 0
    variable = disp_x
    through_thickness_order = SECOND
  [../]
  [./solid_disp_y]
    type = ADStressDivergenceShell
    block = '0'
    component = 1
    variable = disp_y
    through_thickness_order = SECOND
  [../]
  [./solid_disp_z]
    type = ADStressDivergenceShell
    block = '0'
    component = 2
    variable = disp_z
    through_thickness_order = SECOND
  [../]
  [./solid_rot_x]
    type = ADStressDivergenceShell
    block = '0'
    component = 3
    variable = rot_x
    through_thickness_order = SECOND
  [../]
  [./solid_rot_y]
    type = ADStressDivergenceShell
    block = '0'
    component = 4
    variable = rot_y
    through_thickness_order = SECOND
  [../]
[]

[Materials]
  [./elasticity]
    type = ADComputeIsotropicElasticityTensorShell
    youngs_modulus = 1e3
    poissons_ratio = 0.3
    block = 0
    through_thickness_order = SECOND
  [../]
  [./strain]
    type = ADComputeIncrementalShellStrain
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    thickness = 1.0
    through_thickness_order = SECOND
  [../]
  [./stress]
    type = ADComputeShellStress
    block = 0
    through_thickness_order = SECOND
  [../]
[]

[Postprocessors]
  [./disp_z2]
    type = PointValue
    point = '40.0 20.0 0.0'
    variable = disp_z
  [../]
[]

[Outputs]
  exodus = true
[]
