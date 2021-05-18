[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmax = 2
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1.0 0 0'
    block_id = 1
    top_right = '2.0 1.0 0'
  []
  [interface_primary]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary_interface'
  []
[]

[Variables]
  [u]
    type = MooseVariableFVReal
    block = 0
    initial_condition = 0.5
  []
  [v]
    type = MooseVariableFVReal
    block = 1
    initial_condition = 0.5
  []
[]

[FVKernels]
  [diff_left]
    type = FVDiffusion
    variable = u
    coeff = 'left'
    block = 0
  []
  [gradient_creating]
    type = FVBodyForce
    variable = u
  []
  [diff_right]
    type = FVDiffusion
    variable = v
    coeff = 'right'
    block = 1
  []
  [gradient_creating_2]
    type = FVBodyForce
    variable = v
  []
[]

[FVInterfaceKernels]
  [interface]
    type = FVDiffusionInterface
    variable1 = u
    variable2 = v
    boundary = 'primary_interface'
    subdomain1 = '0'
    subdomain2 = '1'
    coeff1 = 'left'
    coeff2 = 'right'
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = u
    boundary = 'left'
    value = 1
  []
  [v_left]
    type = FVDirichletBC
    variable = v
    boundary = 'right'
    value = 0
  []
[]

[Materials]
  [block0]
    type = ADGenericConstantMaterial
    block = '0'
    prop_names = 'left'
    prop_values = '1'
  []
  [block1]
    type = ADGenericConstantMaterial
    block = '1'
    prop_names = 'right'
    prop_values = '1'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
