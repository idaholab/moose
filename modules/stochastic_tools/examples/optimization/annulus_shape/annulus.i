inner_radius = 6
thickness = 4

[Mesh]
  [disk]
    type = ConcentricCircleMeshGenerator
    has_outer_square = false
    preserve_volumes = false
    radii = '${inner_radius} ${fparse inner_radius + thickness}'
    rings = '16 16'
    num_sectors = 16
  []
  [ring]
    type = BlockDeletionGenerator
    input = disk
    block = 1
    new_boundary = inner
  []
[]

[Variables]
  [T]
  []
[]

[Kernels]
  [diffusion]
    type = ADMatDiffusion
    variable = T
    diffusivity = k
  []
  [src]
    type = ADBodyForce
    variable = T
    value = 1
  []
[]

[BCs]
  [convection]
    type = ADMatNeumannBC
    boundary = inner
    variable = T
    boundary_material = convection
    value = 1
  []
[]

[Materials]
  [conductivity]
    type = ADGenericConstantMaterial
    prop_names = 'k'
    prop_values = '1'
  []
  [convection]
    type = ADParsedMaterial
    expression = 'h * (100 - T)'
    coupled_variables = 'T'
    constant_names = 'h'
    constant_expressions = '${fparse 10 / (pi * inner_radius^3)}'
    property_name = convection
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  line_search = none
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre    boomeramg'
[]

[Postprocessors]
  [Tmax]
    type = NodalExtremeValue
    variable = T
  []
  [volume]
    type = VolumePostprocessor
  []
[]

