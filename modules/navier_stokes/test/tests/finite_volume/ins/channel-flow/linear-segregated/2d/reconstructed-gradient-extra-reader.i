!include 2d-velocity-pressure.i

[AuxVariables]
  [ordinary_pressure_gradient]
    order = CONSTANT
    family = MONOMIAL_VEC
  []
  [ordinary_pressure_gradient_x]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [ordinary_pressure_gradient]
    type = FunctorElementalGradientAux
    variable = ordinary_pressure_gradient
    functor = pressure
    execute_on = TIMESTEP_END
  []
  [ordinary_pressure_gradient_x]
    type = VectorVariableComponentAux
    variable = ordinary_pressure_gradient_x
    vector_variable = ordinary_pressure_gradient
    component = x
    execute_on = TIMESTEP_END
  []
[]
