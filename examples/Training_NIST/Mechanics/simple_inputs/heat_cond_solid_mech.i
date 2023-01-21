[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Variables]
  [temperature]
  []
  [disp_x]
  []
  [disp_y]
  []
[]

[Functions]
  [source]
    type = PiecewiseLinear
    x = '0 1 2'
    y = '0 1 0'
  []
[]

[Modules/TensorMechanics/Master]
  [block]
    block = 0
    add_variables = false
    strain = FINITE
    eigenstrain_names = thermal_eigenstrain
    temperature = temperature
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temperature
  []
  [heat_source]
    type = HeatSource
    variable = temperature
    value = 10000
  []
[]

[Materials]
  [heat_conductor]
    type = HeatConductionMaterial
    thermal_conductivity = 1
    block = 0
  []
  [elasticity_tensor1]
    type = ComputeIsotropicElasticityTensor
    block = 0
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [thermal_expansion_strain1]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 200
    thermal_expansion_coeff = 1.0e-4
    temperature = temperature
    eigenstrain_name = thermal_eigenstrain
    block = 0
  []
  [stress1]
    type = ComputeFiniteStrainElasticStress
    block = 0
  []
[]

[BCs]
  [leftright_temp]
    type = DirichletBC
    variable = temperature
    boundary = 'left right'
    value = 200
  []
  [leftright_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left right'
    value = 0
  []
  [bottom_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'

  dt = 0.1
  dtmin = 0.01
  end_time = 1.0

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8

  l_max_its = 100
  nl_max_its = 25
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [peak_temp]
    type = NodalExtremeValue
    variable = temperature
  []
[]
