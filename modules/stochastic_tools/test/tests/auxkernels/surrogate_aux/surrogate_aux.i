[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    ny = 10
    nz = 10
  []
[]

[Surrogates]
  [surrogate]
    type = PolynomialRegressionSurrogate
    filename = surrogate_trainer_poly_regression.rd
  []
[]

[AuxVariables]
  [u]
    family = MONOMIAL
    order = CONSTANT
  []
  [var]
    family = MONOMIAL
    order = CONSTANT
  []
  [reference]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [var_ic]
    type = FunctionIC
    variable = var
    function = funz
  []
[]

[Functions]
  [funx]
    type = ParsedFunction
    expression = 'x'
  []
  [funz]
    type = ParsedFunction
    expression = 'z'
  []
  [funt]
    type = ParsedFunction
    expression = 't'
  []

  [reference]
    type = ParsedFunction
    expression = '1 +   x +   c +   z +   t +
                      x*x + x*c + x*z + x*t +
                            c*c + c*z + c*t +
                                  z*z + z*t +
                                        t*t'
    symbol_names = c
    symbol_values = 3.14
  []
[]

[Postprocessors]
  [pp]
    type = FunctionValuePostprocessor
    function = funt
    point = '0 0 0'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[AuxKernels]
  [u_aux]
    type = SurrogateModelAuxKernel
    variable = u
    model = surrogate
    parameters = 'funx 3.14 var pp'
    scalar_parameters = 'funx pp'
    coupled_variables = 'var'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Postprocessors]
  [diff]
    type = ElementL2Error
    variable = u
    function = reference
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 10
[]

[UserObjects]
  [terminator]
    type = Terminator
    expression = 'diff > 1e-8'
    error_level = ERROR
  []
[]
