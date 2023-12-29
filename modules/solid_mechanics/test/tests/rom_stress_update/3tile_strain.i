# This is a test to check that changing the finite_difference_width does indeed change convergence
# The number of nonlinear iterations should be greater a width of 1e-20 than 1e-2

[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Problem]
  type = ReferenceResidualProblem
  reference_vector = 'ref'
  extra_tag_vectors = 'ref'
  group_variables = 'disp_x disp_y disp_z'
[]

[AuxVariables]
  [temperature]
    initial_condition = 920
  []
[]

[AuxKernels]
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    add_variables = true
    generate_output = 'vonmises_stress'
    extra_vector_tags = 'ref'
  []
[]

[BCs]
  [symmx]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [symmy]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [symmz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
  [pull_x]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 5e-4
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e11
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = rom_stress_prediction
  []
  [rom_stress_prediction]
    type = LAROMANCE3TileTest
    temperature = temperature
    outputs = all
    initial_cell_dislocation_density = 5.7e12
    initial_wall_dislocation_density = 4.83e11
  []
[]

[Executioner]
  type = Transient

  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  dt = 1e-5
  num_steps = 5
[]

[Postprocessors]
  [extrapolation]
    type = ElementAverageValue
    variable = ROM_extrapolation
    outputs = console
  []
  [temperature]
    type = ElementAverageValue
    variable = temperature
    outputs = 'console'
  []
  [partition_weight]
    type = ElementAverageMaterialProperty
    mat_prop = partition_weight
    outputs = 'console'
  []
  [creep_rate]
    type = ElementAverageMaterialProperty
    mat_prop = creep_rate
  []
  [rhom_rate]
    type = ElementAverageMaterialProperty
    mat_prop = cell_dislocation_rate
    outputs = 'console'
  []
  [rhoi_rate]
    type = ElementAverageMaterialProperty
    mat_prop = wall_dislocation_rate
    outputs = 'console'
  []
  [vonmises]
    type = ElementAverageValue
    variable = vonmises_stress
    outputs = 'console'
  []
  [nl_its]
    type = NumNonlinearIterations
    outputs = none
  []
  [total_nl_its]
    type = CumulativeValuePostprocessor
    postprocessor = nl_its
    outputs = 'console'
  []
[]

[Outputs]
  csv = true
  perf_graph = true
[]
