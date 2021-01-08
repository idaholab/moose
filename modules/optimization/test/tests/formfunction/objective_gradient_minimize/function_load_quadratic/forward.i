
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmax = 2
  ymax = 2
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
    value = alpha*x*x+beta*beta*x+c
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
    type = MeasuredDataPointSampler
    variable = temperature
    points = '0.2 0.5 0
              0.5 0.5 0
              1.5 0.5 0
              1.8 0.5 0'
    measured_values = '210 220 160 120'
  []
[]

[Outputs]
  # console = true
  exodus = true
  # csv = true
  file_base = 'forward'
[]
