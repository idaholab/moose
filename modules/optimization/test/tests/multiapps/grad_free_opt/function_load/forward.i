
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
    value = 200
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = top
    value = 100
  []
[]

[Functions]
  [volumetric_heat_func]
    type = ParsedFunction
    value = alpha*sin(C1+x*pi/2)*sin(C2+y*pi/2)+beta
    vars = 'alpha beta C1 C2'
    vals = '100 1 -10 -10'
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
    type = PointValueSampler
    points = '0.2 0.2 0
              0.8 0.6 0
              0.2 1.4 0
              0.8 1.8 0'
    variable = temperature
    sort_by = y
  []
[]

[Outputs]
  console = true
  exodus = true
  file_base = 'forward'
[]
