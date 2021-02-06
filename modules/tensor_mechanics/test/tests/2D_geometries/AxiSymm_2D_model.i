[Mesh]
  file = gold/AxiSymm_trial_model.e
  construct_side_list_from_node_list = true
  # block 1 surface 1
  # block 2 curve 5 to 8
  #
  # nodeset 1 add curve 1		# top
  # nodeset 2 add curve 2		# left
  # nodeset 3 add curve 3		# bot
  # nodeset 4 add curve 4		# right
[]

[GlobalParams]
  displacements = 'disp_r disp_z'
  volumetric_locking_correction = true
[]

[Problem]
  coord_type = RZ
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
  [../]
[]

[AuxVariables]
  [./temperature]
    initial_condition = 298.0
  [../]
[]

[BCs]
  [./symmetry_x]
    type = DirichletBC
    variable = disp_r
    value = 0
    boundary = '4'
  [../]
  [./roller_z]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = '3'
  [../]
  [./top_load]
    type = FunctionDirichletBC
    variable = disp_z
    function = -0.001*t
    boundary = '1'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.3
  [../]
  [./_elastic_strain]
    type = ComputeFiniteStrainElasticStress
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  line_search = 'none'

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  nl_max_its = 15

  l_tol = 1e-6
  l_max_its = 50

  start_time = 0.0
  end_time = 1
  dt = 0.1
[]

[Postprocessors]
  [./center_temperature]
    type = AxisymmetricCenterlineAverageValue
    variable = temperature
    boundary = '4'
  [../]
[]

[Outputs]
  csv = true
  perf_graph = true
[]
