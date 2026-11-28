# NEML2 Salehani-Irani 3DC traction-separation law counterpart for the shifted
# cohesive zone method. Mirrors the native MOOSE SalehaniIrani3DCTraction material
# used by rectangle_sczm_tri_material.i.
[Models]
  [decompose]
    type = VecComponents
    from = 'interface_displacement_jump'
    to = 'normal_separation tangential_separation_1 tangential_separation_2'
  []
  [traction]
    type = SalehaniIraniTraction
    traction = 'interface_traction'
    normal_characteristic_length = 1.0
    tangential_characteristic_length = 0.5
    normal_strength = 500.0
    shear_strength = 300.0
    viscosity = 0.0
  []
  [czm]
    type = ComposedModel
    models = 'decompose traction'
    additional_outputs = 'damage'
  []
[]
