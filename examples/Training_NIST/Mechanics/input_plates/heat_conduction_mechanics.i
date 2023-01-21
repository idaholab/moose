[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
   file = heat_conduction_out.e
  [fmesh]
    type = FileMeshGenerator
    file = 'mesh_in.e'
  []
[]

[Variables]
  [temperature]
    initial_from_file_var = temperature
  []
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Modules/TensorMechanics/Master]
  [block]
    block = 'fuel clad'
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
    block = fuel
    variable = temperature
    value = 1e9
  []
[]

[Materials]
  [heat_conductor_fuel]
    type = HeatConductionMaterial
    thermal_conductivity = 0.01
    block = 'fuel'
  []
  [heat_conductor_clad]
    type = HeatConductionMaterial
    thermal_conductivity = 1
    block = 'clad'
  []
  [elasticity_tensor1]
    type = ComputeIsotropicElasticityTensor
    block = 'fuel clad'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [thermal_expansion_strain1]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 200
    thermal_expansion_coeff = 1.0e-4
    temperature = temperature
    eigenstrain_name = thermal_eigenstrain
    block = 'fuel clad'
  []
  [stress1]
    type = ComputeFiniteStrainElasticStress
    block = 'fuel clad'
  []
[]

[BCs]
  [walls]
    type = DirichletBC
    variable = temperature
    boundary = 'clad_wall'
    value = 300
  []
  [leftright_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'bottom_to_clad top_to_clad'
    value = 0
  []
  [bottom_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom_to_clad top_to_clad'
    value = 0
  []
  [bottom_disp_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'bottom_to_clad top_to_clad'
    value = 0
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'

  nl_abs_tol = 1e-6
  l_abs_tol = 1e-7

  l_max_its = 50
  nl_max_its = 25
[]


[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]

  [peak_temperature]
    type = NodalExtremeValue
    variable = temperature
  []

  [max_displacement_x]
    type = NodalExtremeValue
    variable = disp_x
    value_type = min
  []

  [max_displacement_y]
    type = NodalExtremeValue
    variable = disp_y
  []

  [max_displacement_z]
    type = NodalExtremeValue
    variable = disp_z
  []
[]


