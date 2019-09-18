[Mesh]
  [file]
    type = FileMeshGenerator
    file = 2subdomains.e
  []
  [boundary_fuel_side]
    input = file
    type = SubdomainBoundingBoxGenerator
    block_id = 2
    bottom_left = '0.2 0 0'
    top_right = '0.3 1 0'
  []
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [elemental]
    block = '2'
    order = CONSTANT
    family = MONOMIAL
  []
  [nodal]
    block = '2'
  []
[]


[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[AuxKernels]
  [elemaux]
    type = CoupledAux
    variable = elemental
    coupled = u
    block = '2'
  []
  [nodaux]
    type = CoupledAux
    variable = nodal
    coupled = u
    block = '2'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
