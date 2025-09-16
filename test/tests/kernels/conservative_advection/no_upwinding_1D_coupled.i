# ConservativeAdvection with upwinding_type = None
# Coupled a variable of (1, 0, 0) and see a pulse advect to the right
# Note there are overshoots and undershoots

!include no_upwinding_1D.i

[Variables]
  [coupled]
  []
[]

[ICs]
  [coupled]
    type = FunctionIC
    function = x
    variable = coupled
  []
[]

[Kernels]
  [coupled_udot]
    type = TimeDerivative
    variable = coupled
  []
  [coupled_kernel]
    type = CoupledForce
    v = 1
    coef = 1
    variable = coupled
  []
  [advection]
    velocity_as_variable_gradient = coupled
  []
[]

[Executioner]
  solve_type := NEWTON
[]
