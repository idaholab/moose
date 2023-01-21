[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./element_l2_error]
    # Aux field variable representing the L2 error on each element
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./element_h1_error]
    # Aux field variable representing the H1 error on each element
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./element_l2_norm]
    # Aux field variable representing the L^2 norm of the solution variable
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    expression = sin(2*pi*x)*sin(2*pi*y)
  [../]

  [./forcing_fn]
    type = ParsedFunction
    expression = 8*pi^2*sin(2*pi*x)*sin(2*pi*y)
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./ffn]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[AuxKernels]
  [./l2_norm_aux]
    type = ElementLpNormAux
    variable = element_l2_norm
    coupled_variable = u
  [../]
  [./l2_error_aux]
    type = ElementL2ErrorFunctionAux
    variable = element_l2_error
    # A function representing the exact solution for the solution
    function = exact_fn
    # The nonlinear variable representing the FEM solution
    coupled_variable = u
  [../]
  [./h1_error_aux]
    type = ElementH1ErrorFunctionAux
    variable = element_h1_error
    # A function representing the exact solution for the solution
    function = exact_fn
    # The nonlinear variable representing the FEM solution
    coupled_variable = u
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = 'bottom left right top'
    function = exact_fn
  [../]
[]

[Postprocessors]
  [./L2_error]
    # The L2 norm of the error over the entire mesh.  Note: this is
    # *not* equal to the sum over all the elements of the L2-error
    # norms.
    type = ElementL2Error
    variable = u
    function = exact_fn
  [../]
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
