# Frictional 3D mortar contact with the physical contact constants. The top block's element heights
# vary along the contact face, so the derived contact stiffness scale varies too, and the top block
# slides while it is pressed into the bottom block.
!include varying_derived_scale.i

[Contact]
  [mortar]
    model := coulomb
    friction_coefficient = 0.4
    friction_projection_degree = ONE
  []
[]

[BCs]
  [topx]
    function := '0.05 * t'
  []
[]

[Executioner]
  num_steps := 3
[]

[Postprocessors]
  [contact_pressure_integral]
    type = SideIntegralVariablePostprocessor
    variable = contact_pressure
    boundary = 'top_bottom'
  []
  [tangential_pressure_x_integral]
    type = SideIntegralVariablePostprocessor
    variable = mortar_tangent_x
    boundary = 'top_bottom'
  []
[]
