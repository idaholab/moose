# This input was used for generating the gold file

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 3
  ny = 3
  elem_type = QUAD4
[]

[Variables]
  [u]
    family = LAGRANGE
    order = FIRST
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [scalar]
    type = BodyForce
    variable = u
    value = 10
  []
[]

[BCs]
  [left]
    type = NeumannBC
    variable = u
    boundary = '3'
    value = 10
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = '1'
    value = 0
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
    solve_type = 'PJFNK'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  file_base = kokkos_scalar_constraint_out
[]
