# example similar to:
#https://mooseframework.inl.gov/getting_started/examples_and_tutorials/tutorial03_verification/step04_mms.html
#units kg,m,s,K
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  xmin = -50e-3
  ymin = -50e-3
  xmax = 50e-3
  ymax = 50e-3
[]

[Variables]
  [temperature]
  []
[]

[ICs]
  [T_O]
    type = ConstantIC
    variable = temperature
    value = 20
  []
[]

[Kernels]
  [T_time]
    type = HeatConductionTimeDerivative
    variable = temperature
    density_name = 2000
    specific_heat = 50
  []
  [T_cond]
    type = HeatConduction
    variable = temperature
    diffusion_coefficient = 6
  []
  [T_source]
    type = HeatSource
    variable = temperature
    function = source
  []
[]

[Functions]
  [source]
    type = ParsedFunction
    vars = 'A xo yo sx sy'
    vals = '1e4 0e-3 0e-3  20e-3 20e-3'
    value = 'A*exp(-((x-xo)*(x-xo)/2/sx/sx+(y-yo)*(y-yo)/2/sy/sy))*t'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = temperature
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = top
    value = 0
  []
[]

# [Materials]
#   [steel]
#     type = ADGenericConstantMaterial
#     prop_names = thermal_conductivity
#     prop_values = 5
#   []
#   [volumetric_heat]
#     type = ADGenericFunctionMaterial
#     prop_names = 'volumetric_heat'
#     prop_values = volumetric_heat_func
#   []
# []

[Executioner]
  type = Transient
  solve_type = NEWTON
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  dt = 1
  end_time = 50
[]

[VectorPostprocessors]
  #   [data_pt]
  #     type = VppPointValueSampler
  #     variable = temperature
  #     reporter_name = measure_data
  #   []
  #Zach  This would probably be what the measurement data.
  #can this be output to json?
  [horizontal]
    type = LineValueSampler
    variable = 'temperature'
    start_point = '-50e-3 0 0'
    end_point = '50e-3 0 0'
    num_points = 100
    sort_by = x
    outputs = json
  []
[]

# [Reporters]
#   [measure_data]
#     type = OptimizationData
#   []
# []

[Outputs]
  # console = false
  # json = true
  exodus = true
  file_base = 'forward'
  # [json]
  #   type = JSON
  #   execute_system_information_on = none
  #   #file_base = 'var_in'
  # []
[]
