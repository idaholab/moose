[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1.5 2.4 0.1'
    dy = '1.3 0.9'
    ix = '2 1 1'
    iy = '2 3'
    subdomain_id = '0 1 1 2 2 2'
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
    coeff = 1
  []

  [adv]
    type = FVMatAdvection
    variable = u
    vel = v_mat
  []

  [body_force]
    type = FVBodyForce
    variable = u
    value = 10
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
    value = 1
  []

  [top]
    type = FVNeumannBC
    variable = u
    value = 1
    boundary = 'top'
  []
[]

[Materials]
  [v_mat]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'v_mat'
    prop_values = '4 0 0'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
