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
  [interface]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  []
[]

[Variables]
  [u]
    type = MooseVariableFVReal
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
[]

[FVInterfaceKernels]
  [interface]
    type = FVOneVarDiffusionInterface
    variable1 = u
    boundary = primary0_interface
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
