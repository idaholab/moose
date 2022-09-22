[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '10 10'
    ix = '2 2'
    dy = '20'
    iy = '4'
    subdomain_id = '1 2'
  []
[]

[Variables]
  [v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []

  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [fem_diff1]
    type = AnisotropicDiffusion
    variable = u
    tensor_coeff = '1 0 0
                    0 10 0
                    0 0 0'
    block = 1
  []

  [fem_diff2]
    type = AnisotropicDiffusion
    variable = u
    tensor_coeff = '10 0 0
                    0 10 0
                    0 0 0'
    block = 2
  []
[]

[BCs]
  [fem_left_bottom]
    type = NeumannBC
    variable = u
    boundary = 'left bottom'
    value = 1
  []
  [fem_top_right]
    type = DirichletBC
    variable = u
    boundary = 'right top'
    value = 0
  []
[]

[FVKernels]
  [diff]
    type = FVAnisotropicDiffusion
    variable = v
    coeff = coeff
  []
[]

[FVBCs]
  [left_bottom]
    type = FVNeumannBC
    variable = v
    boundary = 'left bottom'
    value = 1
  []
  [top_right]
    type = FVDirichletBC
    variable = v
    boundary = 'right top'
    value = 0
  []
[]

[Materials]
  [diff1]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'coeff'
    prop_values = '1 10 1'
    block = 1
  []

  [diff2]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'coeff'
    prop_values = '10 10 1'
    block = 2
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
