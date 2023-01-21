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

[Functions]
    [rho_UMo]
        type = ParsedFunction
        value = '-0.9215 * x + 17409'
    []
    [cp_UMo]
        type = ParsedFunction
        value = '0.0692 * x + 113.61'
    []
    [k_UMo]
        type = ParsedFunction
        value = '0.0413 * x + 0.1621'
    []

    [rho_Al]
        type = ParsedFunction
        value = '2702'
    []
    [cp_Al]
        type = ParsedFunction
        value = '3.97e-5 * x * x + 0.41 * x + 773'
    []
    [k_Al]
        type = ParsedFunction
        value = '-1.73e-7 * x * x * x + 2.66e-5 * x * x + 0.16 * x + 120.6'
    []
[]

[SolidProperties]
    [UMo_thermal]
        type = ThermalFunctionSolidProperties
        k = k_UMo
        cp = cp_UMo
        rho = rho_UMo
    []
    [Al_thermal]
        type = ThermalFunctionSolidProperties
        k = k_Al
        cp = cp_Al
        rho = rho_Al
    []
[]

[Materials]
    [UMo_thermal]
        type = ThermalSolidPropertiesMaterial
        sp = 'UMo_thermal'
        temperature = temperature
        block = 'fuel'
    []
    [Al_thermal]
        type = ThermalSolidPropertiesMaterial
        sp = 'Al_thermal'
        temperature = temperature
        block = 'clad'
    []
    [UMo_mechanical]
        type = ComputeIsotropicElasticityTensor
        poissons_ratio = 0.41
        youngs_modulus = 88.4e9
        block = 'fuel'
    []
    [Al_mechanical]
        type = ComputeIsotropicElasticityTensor
        poissons_ratio = 0.33
        youngs_modulus = 68.3e9
        block = 'clad'
    []
    [thermal_expansion_fuel]
        type = ComputeThermalExpansionEigenstrain
        stress_free_temperature = 300
        thermal_expansion_coeff = 13e-6
        temperature = temperature
        eigenstrain_name = thermal_eigenstrain
        block = 'fuel'
    []
    [thermal_expansion_clad]
        type = ComputeThermalExpansionEigenstrain
        stress_free_temperature = 300
        thermal_expansion_coeff = 23e-6
        temperature = temperature
        eigenstrain_name = thermal_eigenstrain
        block = 'clad'
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

  l_max_its = 200
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