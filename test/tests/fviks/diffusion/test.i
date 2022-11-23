L = 2
l = 1
q1 = 1
q2 = 2
uR = 1
D1 = 1
D2 = 2

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmax = ${L}
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '${l} 0 0'
    block_id = 1
    top_right = '${L} 1.0 0'
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
    coeff_interp_method = average
  []
  [source_left]
    type = FVBodyForce
    variable = u
    function = ${q1}
    block = 0
  []
  [diff_right]
    type = FVDiffusion
    variable = v
    coeff = 'right'
    block = 1
    coeff_interp_method = average
  []
  [source_right]
    type = FVBodyForce
    variable = v
    function = ${q2}
    block = 1
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
    coeff_interp_method = average
  []
[]

[FVBCs]
  [v_left]
    type = FVDirichletBC
    variable = v
    boundary = 'right'
    value = ${uR}
  []
[]

[Materials]
  [block0]
    type = ADGenericFunctorMaterial
    block = '0'
    prop_names = 'left'
    prop_values = '${D1}'
  []
  [block1]
    type = ADGenericFunctorMaterial
    block = '1'
    prop_names = 'right'
    prop_values = '${D2}'
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
