# ConservativeAdvection with upwinding_type = None
# Coupled an aux of (1, 0, 0) and see a pulse advect to the right
# Note there are overshoots and undershoots

!include no_upwinding_1D.i

[AuxVariables]
  [coupled]
  []
[]

[AuxKernels]
  [coupled_aux]
    type = ParsedAux
    expression = 'x * 2'
    variable = coupled
    use_xyzt = true
  []
[]

[Materials]
  [scalar]
    type = GenericConstantMaterial
    prop_names = scalar
    prop_values = 0.5
  []
[]

[Kernels]
  [advection]
    velocity_as_variable_gradient = coupled
    velocity_scalar_coef = scalar
  []
[]

[Outputs]
  hide = coupled
[]
