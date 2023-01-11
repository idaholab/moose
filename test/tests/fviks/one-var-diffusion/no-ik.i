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
[]

[Variables]
  [u]
    type = MooseVariableFVReal
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = u
    coeff = 'coeff'
    coeff_interp_method = average
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
    type = ADGenericFunctorMaterial
    block = '0'
    prop_names = 'coeff'
    prop_values = '4'
  []
  [block1]
    type = ADGenericFunctorMaterial
    block = '1'
    prop_names = 'coeff'
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
  csv = true
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    expression = 'if(x<1, 1 - x/3, 4/3 - 2*x/3)'
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
