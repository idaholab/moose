# Test for small strain Euler beam bending in y direction

# Modeling a tube with an outer radius of 15 mm and inner radius of 13 mm
# The length of the tube is 1.0 m
# E = 2.068e11 Pa and G = 7.956e10 with nu = 0.3
# A load of 5 N is applied at the end of the beam in the y-dir

# The displacement at the end is given by
# y = - W * L^3 / 3 * E * I
# y = - 5 * 1.0^3 / 3 * 2.068e11 * 1.7329e-8 = 4.65e-4 m

# where I = pi/2 * (r_o^4 - r_i^4)
# I = pi /2 * (0.015^4 - 0.013^4) = 1.7329e-8

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0.0
  xmax = 1.0
  displacements = 'disp_x disp_y disp_z'
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
  [./rot_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0.0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = left
    value = 0.0
  [../]
  [./fixr1]
    type = DirichletBC
    variable = rot_x
    boundary = left
    value = 0.0
  [../]
  [./fixr2]
    type = DirichletBC
    variable = rot_y
    boundary = left
    value = 0.0
  [../]
  [./fixr3]
    type = DirichletBC
    variable = rot_z
    boundary = left
    value = 0.0
  [../]
[]

[NodalKernels]
  [./force_y2]
    type = ConstantRate
    variable = disp_y
    boundary = right
    rate = 5.0
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
  solve_type = PJFNK
  line_search = 'none'
  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8

  dt = 1
  dtmin = 1
  end_time = 2
[]

[Kernels]
  [./solid_disp_x]
    type = StressDivergenceBeam
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 0
    variable = disp_x
  [../]
  [./solid_disp_y]
    type = StressDivergenceBeam
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 1
    variable = disp_y
  [../]
  [./solid_disp_z]
    type = StressDivergenceBeam
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 2
    variable = disp_z
  [../]
  [./solid_rot_x]
    type = StressDivergenceBeam
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 3
    variable = rot_x
  [../]
  [./solid_rot_y]
    type = StressDivergenceBeam
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 4
    variable = rot_y
  [../]
  [./solid_rot_z]
    type = StressDivergenceBeam
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 5
    variable = rot_z
  [../]
[]

[Materials]
  [./elasticity]
    type = ComputeElasticityBeam
    youngs_modulus = 2.068e11
    poissons_ratio = 0.3
    shear_coefficient = 1.0
    block = 0
    outputs = exodus
    output_properties = 'material_stiffness material_flexure'
  [../]
  [./strain]
    type = ComputeIncrementalBeamStrain
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    area = 1.759e-4
    Ay = 0.0
    Az = 0.0
    Iy = 1.7329e-8
    Iz = 1.7329e-8
    y_orientation = '0.0 1.0 0.0'
  [../]
  [./stress]
    type = ComputeBeamResultants
    block = 0
    outputs = exodus
    output_properties = 'forces moments'
  [../]
[]

[Postprocessors]
  [./disp_x]
    type = PointValue
    point = '1.0 0.0 0.0'
    variable = disp_x
  [../]
  [./disp_y]
    type = PointValue
    point = '1.0 0.0 0.0'
    variable = disp_y
  [../]
  [./forces_y]
    type = PointValue
    point = '1.0 0.0 0.0'
    variable = forces_y
  [../]
[]

[Outputs]
  csv = true
  exodus = true
[]
