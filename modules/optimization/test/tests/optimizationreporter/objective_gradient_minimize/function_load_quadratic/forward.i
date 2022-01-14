
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmax = 2
  ymax = 2
[]

[AuxVariables]
  [bodyLoad]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [./bodyLoad]
    type = ADMaterialRealAux
    property = volumetric_heat
    variable = bodyLoad
  [../]
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
  []
  [./heat_source]
    type = ADMatHeatSource
    material_property = volumetric_heat
    variable = temperature
  [../]
[]

[BCs]
  [left]
    type = NeumannBC
    variable = temperature
    boundary = left
    value = 0
  []
  [right]
    type = NeumannBC
    variable = temperature
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 100
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = top
    value = 150
  []
[]

[Functions]
  [volumetric_heat_func]
    type = ParsedFunction
    value = alpha*x*x+beta*x+c
    vars = 'alpha beta c'
    vals = '0 0 0'
  []
[]

[Materials]
  [steel]
    type = ADGenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
  []
  [volumetric_heat]
    type = ADGenericFunctionMaterial
    prop_names = 'volumetric_heat'
    prop_values = volumetric_heat_func
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[VectorPostprocessors]
  [data_pt]
    type = VppPointValueSampler
    variable = temperature
    reporter_name = measure_data
  []
  [horizontal]
    type = LineValueSampler
    variable = 'temperature'
    start_point = '0 0.5 0'
    end_point = '2 0.5 0'
    num_points = 21
    sort_by = x
  [../]
  [horizontal2]
    type = LineValueSampler
    variable = 'temperature'
    start_point = '0 1.1 0'
    end_point = '2 1.1 0'
    num_points = 21
    sort_by = x
  [../]
[]

[Reporters]
  [measure_data]
    type=OptimizationData
  []
[]

[Outputs]
  # console = true
  exodus = false
  csv = false
  # file_base = 'forward'
[]
