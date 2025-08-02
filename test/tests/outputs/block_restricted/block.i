[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    subdomain_ids = '0 1 2 3
                     0 0 1 1
                     2 2 2 3
                     3 3 2 1'
  []
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [v]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = FunctionAux
      function = 'x'
    []
  []
  [ignore]
  []
  [going_to_be_ignored_due_to_block_restriction]
    block = '0 1'
  []
[]

[Materials]
  [generic]
    type = GenericConstantMaterial
    prop_names = 'mat'
    prop_values = '12'
    outputs = 'all'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  [out]
    type = Exodus
    sampling_blocks = '2 3'
  []
[]
