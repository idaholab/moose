[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Debug]
  show_material_props = true
[]

[MeshModifiers]
  [./subdomain1]
    type = SubdomainBoundingBox
    bottom_left = '0 0 0'
    top_right = '0.5 0.5 0'
    block_id = 1
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./force]
    type = BodyForce
    variable = u
  [../]
[]

[BCs]
  [./vacuum]
    type = DGMDDBC
    variable = u
    boundary = 'left right top bottom'
    epsilon = 1
    sigma = 4
    prop_name = prop
    function = zero
  [../]
[]

[Functions]
  [./zero]
    type = ParsedFunction
    value = 0
  [../]
[]

[Materials]
  [./mat1]
    type = GenericConstantMaterial
# missing block 0 here, we should see an error!
    block = '1'
    prop_names = 'prop'
    prop_values = '0'
  [../]
  [./dummy]
    type = GenericConstantMaterial
    block = '0 1'
    prop_names = 'dummy'
    prop_values = '1'
  [../]
[]

[Postprocessors]
  [./unorm]
    type = ElementL2Norm
    variable = u
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
