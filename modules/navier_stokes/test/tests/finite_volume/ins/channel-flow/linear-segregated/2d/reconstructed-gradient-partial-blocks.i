!include momentum-pressure-block-restricted.i

[FVGradientMethods]
  [reconstructed]
    type = FVReconstructedPressureGradient
    base_gradient_method = green-gauss
    gradient_relaxation = 0.5
  []
[]

[Functions]
  [outside_pressure]
    type = ParsedFunction
    expression = x
  []
[]

[LinearFVKernels]
  [u_outside_reaction]
    type = LinearFVReaction
    variable = vel_x
    block = 2
  []
  [v_outside_reaction]
    type = LinearFVReaction
    variable = vel_y
    block = 2
  []
  [pressure_outside_reaction]
    type = LinearFVReaction
    variable = pressure
    block = 2
  []
  [pressure_outside_source]
    type = LinearFVSource
    variable = pressure
    source_density = outside_pressure
    block = 2
  []
[]

[AuxVariables]
  [coupling_pressure_gradient]
    order = CONSTANT
    family = MONOMIAL_VEC
  []
  [coupling_pressure_gradient_x]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [coupling_pressure_gradient]
    type = FunctorElementalGradientAux
    variable = coupling_pressure_gradient
    functor = pressure
    execute_on = TIMESTEP_END
  []
  [coupling_pressure_gradient_x]
    type = VectorVariableComponentAux
    variable = coupling_pressure_gradient_x
    vector_variable = coupling_pressure_gradient
    component = x
    execute_on = TIMESTEP_END
  []
[]

[VectorPostprocessors]
  [outside_gradient]
    type = ElementValueSampler
    variable = coupling_pressure_gradient_x
    block = 2
    sort_by = id
    execute_on = TIMESTEP_END
  []
[]

[Outputs]
  csv = true
[]
