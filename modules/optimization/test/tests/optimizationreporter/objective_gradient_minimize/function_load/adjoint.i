function_vals = '100 1 -10 -10'

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
  [pt]
    type = OptimizationDataPointSource
    variable = temperature
    points = misfit/measurement_points
    values = misfit/misfit_values
  []
[]

[Reporters]
  [misfit]
    type=OptimizationData
  []
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

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

# FUNCTION FROM FORWARD.i
# [Functions]
#   [volumetric_heat_func]
#     type = ParsedFunction
#     value = alpha*sin(C1+x*pi/2)*sin(C2+y*pi/2)+beta
#     vars = 'alpha beta C1 C2'
#     vals = 'p1 p2 p3 p4'
#   []
# []

[Functions]
  [volumetric_heat_func_deriv_alpha]
    type = ParsedFunction
    value = sin(C1+x*pi/2)*sin(C2+y*pi/2)
    vars = 'alpha beta C1 C2'
    vals = ${function_vals}
  []
  [volumetric_heat_func_deriv_beta]
    type = ParsedFunction
    value = 1
    vars = 'alpha beta C1 C2'
    vals = ${function_vals}
  []
  [volumetric_heat_func_deriv_C1]
    type = ParsedFunction
    value = alpha*cos(C1+x*pi/2)*sin(C2+y*pi/2)
    vars = 'alpha beta C1 C2'
    vals = ${function_vals}
  []
  [volumetric_heat_func_deriv_C2]
    type = ParsedFunction
    value = alpha*sin(C1+x*pi/2)*cos(C2+y*pi/2)
    vars = 'alpha beta C1 C2'
    vals = ${function_vals}
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
    function = volumetric_heat_func_deriv_C1
    variable = temperature
  []
  [adjoint_pt_3]
    type = VariableFunctionElementIntegral
    function = volumetric_heat_func_deriv_C2
    variable = temperature
  []
[]

[VectorPostprocessors]
  [adjoint_pt]
    type = VectorOfPostprocessors
    postprocessors = 'adjoint_pt_0 adjoint_pt_1 adjoint_pt_2 adjoint_pt_3'
  []
[]

[Outputs]
  console = true
  exodus = true
  file_base = 'adjoint'
[]
