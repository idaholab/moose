beta = 0.25
gamma = 0.5

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    ymin = 0
    xmax = 0.2
    ymax = 0.5
    nx = 50
    ny = 150
    elem_type = QUAD4
  []
[]

[Variables]
  [disp_x][]
  [disp_y][]
[]

[AuxVariables]
  [solid_indicator]
    [AuxKernel]
      type = ConstantAux
      variable = solid_indicator
      value = 0.0
      boundary = 'left right top'
      execute_on = 'initial timestep_end'
    []
    initial_condition = 1.0
  []
[]

[Modules/TensorMechanics/DynamicMaster]
  [all]
    add_variables = true
    incremental = true
    strain = FINITE
    decomposition_method = EigenSolution
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 10000.0
    poissons_ratio = 0.3
    use_displaced_mesh = true
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
  [density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '1'
    use_displaced_mesh = true
  []
  [constant_stress]
    type = GenericConstantRankTwoTensor
    tensor_values = '100'
    tensor_name = test_tensor
  []
[]

[BCs]
  [hold_x]
    type = DirichletBC
    boundary = bottom
    variable = disp_x
    value = 0
  []
  [hold_y]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0
  []
  [Pressure]
    [push_left]
      boundary = left
      factor = 100
    []
  []
[]

[Executioner]
  type = Transient
  end_time = 10
  dt = 1e-2
  solve_type = 'NEWTON'
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -pc_factor_shift_type'
  petsc_options_value = 'lu       superlu_dist               NONZERO'
  nl_max_its = 40
  l_max_its = 15
  line_search = 'none'
  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-4
  automatic_scaling = true
  [TimeIntegrator]
    type = NewmarkBeta
    beta = ${beta}
    gamma = ${gamma}
  []
[]

[Outputs]
  exodus = true
[]
