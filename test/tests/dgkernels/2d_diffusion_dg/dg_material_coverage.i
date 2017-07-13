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
    order = FIRST
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = DiffMKernel
    variable = u
    mat_prop = prop1
    offset = 0
  [../]
  [./force]
    type = BodyForce
    variable = u
  [../]
[]

[DGKernels]
  [./dgdiffusion]
    type = DGDiffusion
    variable = u
    epsilon = 1
    sigma = 4
    diff = prop2
  [../]
[]

[BCs]
  [./vacuum]
    type = VacuumBC
    variable = u
    boundary = 'left right top bottom'
  [../]
[]

[Materials]
  [./mat1]
    type = GenericConstantMaterial
    block = '0 1'
    prop_names = 'prop1'
    prop_values = '1'
  [../]
  [./mat2]
    type = GenericConstantMaterial
# missing block 0 here, we should see an error!
    block = '1'
    prop_names = 'prop2'
    prop_values = '2'
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
