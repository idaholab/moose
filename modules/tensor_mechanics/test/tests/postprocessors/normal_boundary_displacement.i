sft = 300

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [generated_mesh]
    type = FileMeshGenerator
    file = inclined_geom.e
  []
[]

[Modules/TensorMechanics/Master/All]
  strain = FINITE
  add_variables = true
  eigenstrain_names = thermal_eigenstrain
[]

[Functions]
  [temp_func]
    type = ParsedFunction
    value = 'r := sqrt(x*x+y*y); zp := 1 - z / 0.92; (300 + 2400 * zp) * (1 - 0.125 * r / 0.03)'
  []

  [expansion_func]
    type = ParsedFunction
    value = '5e-5'
  []
[]


[AuxVariables]
  [temperature]
  []
[]

[ICs]
  [temperature_ic]
    type = FunctionIC
    variable = temperature
    function = temp_func
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = 'disp_y'
    boundary = bottom
    value = 0
  []

  [InclinedNoDisplacementBC]
    [top]
      boundary = top
      penalty = 1e-2
    []
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 77e3
    poissons_ratio = 0.3
  []

  [thermal_eigenstrain]
    type = ComputeMeanThermalExpansionFunctionEigenstrain
    eigenstrain_name = thermal_eigenstrain
    temperature = temperature
    thermal_expansion_function = expansion_func
    thermal_expansion_function_reference_temperature = ${sft}
    stress_free_temperature = ${sft}
  []

  [stress]
    type = ComputeFiniteStrainElasticStress
  []
[]

[Postprocessors]
  [top_area]
    type = AreaPostprocessor
    boundary = top
  []

  [top_0]
    type = NormalBoundaryDisplacement
    value_type = average
    boundary = top
  []

  [top_0b]
    type = NormalBoundaryDisplacement
    value_type = average
    normalize = false
    boundary = top
  []

  [top_1]
    type = NormalBoundaryDisplacement
    value_type = absolute_average
    boundary = top
  []

  [top_1b]
    type = NormalBoundaryDisplacement
    value_type = absolute_average
    normalize = false
    boundary = top
  []

  [top_2]
    type = NormalBoundaryDisplacement
    value_type = max
    boundary = top
  []

  [top_3]
    type = NormalBoundaryDisplacement
    value_type = absolute_max
    boundary = top
  []
[]

[Executioner]
  type = Steady

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'

  automatic_scaling = true

  # controls for linear iterations
  l_max_its = 10
  l_tol = 1e-4

  # controls for nonlinear iterations
  nl_max_its = 100
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Outputs]
  exodus = true
[]
