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
    components = 2
  []
  [u0]
    family = MONOMIAL
    order = CONSTANT
  []
  [u1]
    family = MONOMIAL
    order = CONSTANT
  []
  [array_var]
    family = MONOMIAL
    order = CONSTANT
    components = 2
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
  [array_var_ic]
    type = ArrayFunctionIC
    variable = array_var
    function = 'funx funmx'
  []
  [var_ic]
    type = FunctionIC
    variable = var
    function = funy
  []
[]

[Functions]
  [funx]
    type = ParsedFunction
    expression = 'x'
  []
  [funmx]
    type = ParsedFunction
    expression = '-x'
  []
  [funy]
    type = ParsedFunction
    expression = 'y'
  []
  [funz]
    type = ParsedFunction
    expression = 'z'
  []
  [funt]
    type = ParsedFunction
    expression = 't'
  []

  [reference0]
    type = ParsedFunction
    expression = '1 +   x +   y +   z +   t +
                      x*x + x*y + x*z + x*t +
                            y*y + y*z + y*t +
                                  z*z + z*t +
                                        t*t'
  []
  [reference1]
    type = ParsedFunction
    expression = '1 -   x +   y +   z +   t +
                      x*x - x*y - x*z - x*t +
                            y*y + y*z + y*t +
                                  z*z + z*t +
                                        t*t'
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
    type = SurrogateModelArrayAuxKernel
    variable = u
    model = surrogate
    parameters = 'array_var var funz pp'
    scalar_parameters = 'funz pp'
    coupled_variables = 'var'
    coupled_array_variables = 'array_var'
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [u0_aux]
    type = ArrayVariableComponent
    variable = u0
    array_variable = u
    component = 0
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [u1_aux]
    type = ArrayVariableComponent
    variable = u1
    array_variable = u
    component = 1
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Postprocessors]
  [diff0]
    type = ElementL2Error
    variable = u0
    function = reference0
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [diff1]
    type = ElementL2Error
    variable = u1
    function = reference1
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
    expression = '(diff0 + diff1) > 1e-8'
    error_level = ERROR
  []
[]
