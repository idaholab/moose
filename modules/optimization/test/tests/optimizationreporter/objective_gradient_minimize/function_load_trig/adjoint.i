
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
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Functions]
  [volumetric_heat_func_deriv]
    type = ParsedFunction
    value = sin(x*pi/2)*sin(y*pi/2)
  []
[]

[Postprocessors]
  [adjoint_pt_0]
    # integral of load function gradient w.r.t parameter
    type = VariableFunctionElementIntegral
    function = volumetric_heat_func_deriv
    variable = temperature
  []
[]

[VectorPostprocessors]
  [adjoint_pt]
    type = VectorOfPostprocessors
    postprocessors = 'adjoint_pt_0'
  []
[]

[Outputs]
  console = false
  exodus = false
  file_base = 'adjoint'
[]
