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
  [coeff_mat]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = 'coeff'
    subdomain_to_prop_value = '0 4
                               1 2'
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
