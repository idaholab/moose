[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 50
  []
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[Functions]
  [u0]
    type = ParsedFunction
    expression = 'x*(1-x)'
  []
  [v0]
    type = ParsedFunction
    expression = '1+0.5*sin(2*pi*x)'
  []
[]

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = u0
  []
  [v]
    type = FunctionIC
    variable = v
    function = v0
  []
[]

[Kernels]
  [u_diff]
    type = ADMatDiffusion
    variable = u
    diffusivity = v_diffusivity
  []
  [u_rxn]
    type = ADLambdaU2
    variable = u
    lambda = 20000
  []
  [v_offdiag]
    type = ADCoupledFieldKernel
    variable = v
    u = u
  []
  [v_diag]
    type = ADReaction
    variable = v
  []
  [v_exp_diag]
    type = ADExpUKernel
    variable = v
  []
[]

[Materials]
  [v_diffusivity]
    type = ADParsedMaterial
    property_name = v_diffusivity
    expression = 'v'
    coupled_variables = 'v'
  []
[]

[BCs]
  [left]
    type = ADDirichletBC
    variable = u
    value = 0
    boundary = 'left'
  []
  [right]
    type = ADDirichletBC
    variable = u
    value = 1
    boundary = 'right'
  []
[]

[Convergence]
  [u]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-10
    nl_rel_tol = 0
  []
  [v]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-10
    nl_rel_tol = 0
  []
[]

[Postprocessors]
  [u]
    type = ElementAverageValue
    variable = u
    execute_on = 'final'
  []
  [v]
    type = ElementAverageValue
    variable = v
    execute_on = 'final'
  []
[]

[Outputs]
  console = false
  print_nonlinear_residuals = false
  print_linear_residuals = false
[]
