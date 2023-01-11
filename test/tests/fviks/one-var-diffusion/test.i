L = 2
l = 1
q1 = 1
q2 = 2
uR = 1
D1 = 1
D2 = 2

ul = '${fparse 1/D2*(D2*uR+q2*L*L/2-q2*l*l/2-l*(q2-q1)*L+l*l*(q2-q1))}'

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
  [source_left]
    type = FVBodyForce
    variable = u
    function = ${q1}
    block = 0
  []
  [source_right]
    type = FVBodyForce
    variable = u
    function = ${q2}
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
    coeff_interp_method = average
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
    coeff_interp_method = average
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
    coeff_interp_method = average
  []
  [bad3]
    type = FVOneVarDiffusionInterface
    variable1 = v
    boundary = primary_interface
    subdomain1 = '0'
    subdomain2 = '1'
    coeff1 = 'left'
    coeff2 = 'right'
    coeff_interp_method = average
  []
[]

[FVBCs]
  [right]
    type = FVDirichletBC
    variable = u
    boundary = 'right'
    value = ${uR}
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
  csv = true
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    expression = 'if(x<${l}, 1/${D1}*(${fparse D1*ul+q1*l*l/2}-${fparse q1/2}*x*x),-1/${D2}*(${fparse -D2*ul-q2*l*l/2}+${fparse q2/2}*x*x-${fparse l*(q2-q1)}*x+${fparse l*l*(q2-q1)}))'
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
