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
    type = HeatConduction
    variable = temperature
  []
  [./heat_source]
    type = HeatSource
    function = volumetric_heat_func
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
    type = GenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
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
  exodus = true
  file_base = 'forward'
[]
