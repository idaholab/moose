# Test for multi app vector postprocessor to aux variable transfer

# Master App contains 2 beams, each fixed at one end and oriented
# in the z direction. Each subApp contains a 2D model with the same
# height as the length of the beam. A time varying temperature profile
# is assigned to each 2D model and the resulting yy strain along the
# height of the 2D model is transferred to the beam model using the
# multi app transfer. The resulting displacement of the beam should be
# same as that of the 2D models.

[Mesh]
  type = FileMesh
  file = 2_beams.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
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
    boundary = 1
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = 1
    value = 0.0
  [../]
  [./fixr1]
    type = DirichletBC
    variable = rot_x
    boundary = 1
    value = 0.0
  [../]
  [./fixr2]
    type = DirichletBC
    variable = rot_y
    boundary = 1
    value = 0.0
  [../]
  [./fixr3]
    type = DirichletBC
    variable = rot_z
    boundary = 1
    value = 0.0
  [../]
[]

[Kernels]
  [./solid_disp_x]
    type = StressDivergenceBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 0
    variable = disp_x
  [../]
  [./solid_disp_y]
    type = StressDivergenceBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 1
    variable = disp_y
  [../]
  [./solid_disp_z]
    type = StressDivergenceBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 2
    variable = disp_z
  [../]
  [./solid_rot_x]
    type = StressDivergenceBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 3
    variable = rot_x
  [../]
  [./solid_rot_y]
    type = StressDivergenceBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 4
    variable = rot_y
  [../]
  [./solid_rot_z]
    type = StressDivergenceBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 5
    variable = rot_z
  [../]
[]

[Materials]
  [./elasticity]
    type = ComputeElasticityBeam
    youngs_modulus = 2.60072400269
    shear_modulus = 1.0e4
    shear_coefficient = 0.85
    block = 1
  [../]
  [./strain]
    type = ComputeIncrementalBeamStrain
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    area = 0.554256
    Ay = 0.0
    Az = 0.0
    Iy = 0.0141889
    Iz = 0.0141889
    y_orientation = '1.0 0.0 0.0'
    eigenstrain_names = 'thermal'
  [../]
  [./stress]
    type = ComputeBeamResultants
    block = 1
  [../]
  [./thermal]
    type = ComputeEigenstrainBeamFromVariable
    disp_eigenstrain = 'zero1 zero2 to_var'
    eigenstrain_name = thermal
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  line_search = 'none'

  l_max_its = 50
  nl_max_its = 50
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
  l_tol = 1e-9

  start_time = 0.0
  end_time = 0.075
  dt = 0.0125
  dtmin = 0.0001
[]

[AuxVariables]
  [./to_var]
  [../]
  [./zero1]
  [../]
  [./zero2]
  [../]
[]

[MultiApps]
  [./sub]
    type = TransientMultiApp
    app_type = CombinedApp
    positions = '0.0 0.0 0.0 0.0 0.0 0.0'
    input_files = 'subapp1.i subapp2.i'
  [../]
[]

[Transfers]
  [./fromsub]
    type = VectorPostprocessorTransfer
    direction = from_multiapp
    multi_app = sub
    vector_postprocessor = axial_str
    variable_vector_names = 'axial_strain'
    variables = 'to_var'
    positions = 'positions_file.csv'
  [../]
[]

[Postprocessors]
  [./pos1]
    type = PointValue
    variable = disp_z
    point = '2.0 2.0 0.150080'
  [../]
  [./pos2]
    type = PointValue
    variable = disp_z
    point = '3.0 3.0 0.150080'
  [../]
[]

[Outputs]
  exodus = true
[]
