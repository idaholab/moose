
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
[]

[DiracKernels]
  [./pt0]
    type = ConstantPointSource
    variable = temperature
    value = 10
    point = '0.2 0.5 0'
  [../]
  [./pt1]
    type = ConstantPointSource
    variable = temperature
    value = 10
    point = '0.5 0.5 0'
  [../]
  [./pt2]
    type = ConstantPointSource
    variable = temperature
    value = 10
    point = '1.5 0.5 0'
  [../]
  [./pt3]
    type = ConstantPointSource
    variable = temperature
    value = 10
    point = '1.8 0.5 0'
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
    value = 0
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = top
    value = 0
  []
[]

[Materials]
  [steel]
    type = ADGenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
  []
[]

[Problem]#do we need this
  type = FEProblem
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

# FUNCTION FROM FORWARD.i
# type = ParsedFunction
# value = alpha*x*x+beta*beta*x+c
# vars = 'alpha beta c'
# vals = 'p1 p2 p3'

[Functions]
  [volumetric_heat_func_deriv_alpha]
    type = ParsedFunction
    value = x*x
    vars = 'alpha beta c'
    vals = 'p1 p2 p3'
  []
  [volumetric_heat_func_deriv_beta]
    type = ParsedFunction
    value = 2*beta*x
    vars = 'alpha beta c'
    vals = 'p1 p2 p3'
  []
  [volumetric_heat_func_deriv_c]
    type = ParsedFunction
    value = 1
    vars = 'alpha beta c'
    vals = 'p1 p2 p3'
  []
[]

[Postprocessors]
  # integral of load function gradient w.r.t parameter
  [adjoint_pt_0]
    type = VariableFunctionElementIntegral
    function = volumetric_heat_func_deriv_alpha
    variable = temperature
  []
  [adjoint_pt_1]
    type = VariableFunctionElementIntegral
    function = volumetric_heat_func_deriv_beta
    variable = temperature
  []
  [adjoint_pt_2]
    type = VariableFunctionElementIntegral
    function = volumetric_heat_func_deriv_c
    variable = temperature
  []
  [p1]
    type = ConstantValuePostprocessor
    value = 1
    execute_on = linear #these need to have execute_on linear to get the gradient correct
  []
  [p2]
    type = ConstantValuePostprocessor
    value = 1
    execute_on = linear
  []
  [p3]
    type = ConstantValuePostprocessor
    value = 1
    execute_on = linear
  []
[]


[Controls]
  [adjointReceiver]
    type = ControlsReceiver
  []
  [parameterReceiver]
    type = ControlsReceiver
  []
[]


[Outputs]
  # console = true
  exodus = true
  file_base = 'adjoint'
[]
