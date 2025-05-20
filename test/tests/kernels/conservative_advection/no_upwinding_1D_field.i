# ConservativeAdvection with upwinding_type = None
# Coupled a field of (1, 0, 0) and see a pulse advect to the right
# Note there are overshoots and undershoots

!include no_upwinding_1D.i

[AuxVariables]
  [field]
  []
[]

[AuxKernels]
  [field_aux]
    type = ParsedAux
    expression = 'x * 2'
    variable = field
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
    field_variable = field
    scalar = scalar
  []
[]

[Outputs]
  hide = field
[]
