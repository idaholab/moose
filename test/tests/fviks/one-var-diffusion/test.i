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
  [interface_primary_side]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary_interface'
  []
  [interface_secondary_side]
    input = interface_primary_side
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '1'
    paired_block = '0'
    new_boundary = 'secondary_interface'
  []
[]

[Variables]
  [u]
    type = MooseVariableFVReal
  []
  [v]
    type = MooseVariableFVReal
    block = 0
  []
  [w]
    type = MooseVariableFVReal
    block = 1
  []
[]

[FVKernels]
  [diff_left]
    type = FVDiffusion
    variable = u
    coeff = 'left'
    block = 0
  []
  [diff_right]
    type = FVDiffusion
    variable = u
    coeff = 'right'
    block = 1
  []
  [diff_v]
    type = FVDiffusion
    variable = v
    block = 0
    coeff = 'left'
  []
  [diff_w]
    type = FVDiffusion
    variable = w
    block = 1
    coeff = 'right'
  []
[]

[FVInterfaceKernels]
  active = 'interface'
  [interface]
    type = FVOneVarDiffusionInterface
    variable1 = u
    boundary = primary_interface
    subdomain1 = '0'
    subdomain2 = '1'
    coeff1 = 'left'
    coeff2 = 'right'
  []
  [bad1]
    type = FVOneVarDiffusionInterface
    variable1 = w
    variable2 = u
    boundary = primary_interface
    subdomain1 = '0'
    subdomain2 = '1'
    coeff1 = 'left'
    coeff2 = 'right'
  []
  [bad2]
    type = FVOneVarDiffusionInterface
    variable1 = u
    variable2 = v
    boundary = primary_interface
    subdomain1 = '0'
    subdomain2 = '1'
    coeff1 = 'left'
    coeff2 = 'right'
  []
  [bad3]
    type = FVOneVarDiffusionInterface
    variable1 = v
    boundary = primary_interface
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
  [right]
    type = FVDirichletBC
    variable = u
    boundary = 'right'
    value = 0
  []
  [v_left]
    type = FVDirichletBC
    variable = v
    boundary = 'left'
    value = 1
  []
  [v_right]
    type = FVDirichletBC
    variable = v
    boundary = 'primary_interface'
    value = 0
  []
  [w_left]
    type = FVDirichletBC
    variable = w
    boundary = 'secondary_interface'
    value = 1
  []
  [w_right]
    type = FVDirichletBC
    variable = w
    boundary = 'right'
    value = 0
  []
[]

[Materials]
  [block0]
    type = ADGenericConstantMaterial
    block = '0'
    prop_names = 'left'
    prop_values = '4'
  []
  [block1]
    type = ADGenericConstantMaterial
    block = '1'
    prop_names = 'right'
    prop_values = '2'
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
  csv = true
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    value = 'if(x<1, 1 - x/3, 4/3 - 2*x/3)'
  []
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2u]
    type = ElementL2Error
    variable = u
    function = exact_u
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
