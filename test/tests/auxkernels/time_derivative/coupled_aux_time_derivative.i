[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
[]

[Functions]
  [./f_fn]
    type = ParsedFunction
    expression = t*(x+y)
  [../]
  [./f_dot_fn]
    type = ParsedFunction
    expression = (x+y)
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./l2_proj]
    type = Reaction
    variable = u
  [../]
  [./dck]
    type = DotCouplingKernel
    variable = u
    v = f
  [../]
[]

[AuxVariables]
  [./f]
  [../]

  [./g]
  [../]
[]

[AuxKernels]
  [./f_k]
    type = FunctionAux
    variable = f
    function = f_fn
  [../]

  # We do not allow coupling of time derivatives of aux vars into the aux vars
  [./g_k]
    type = DotCouplingAux
    variable = g
    v = f
  [../]
[]

[Postprocessors]
  [./l2_error]
    type = ElementL2Error
    variable = u
    function = f_dot_fn
  [../]
[]

[Executioner]
  type = Transient

  dt = 0.1
  num_steps = 2

  nl_abs_tol = 1.e-15
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
