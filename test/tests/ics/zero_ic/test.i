# This test makes sure that when people use an initial condition that accesses _zero
# the code does not crash. The "problem" is that InitialCondition uses _qp which for
# nodal variables loops over nodes, rather then q-points.  Thus if people have more
# nodes that q-points (they have to dial a lower q-rule in the Executioner block), the
# code would do an out-of-bounds access and crash.

[Mesh]
  type = GeneratedMesh
  dim = 3
  elem_type = HEX27
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = SECOND
  [../]
[]

[ICs]
  [./ic_u]
    type = ZeroIC
    variable = u
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./all]
    type = DirichletBC
    variable = u
    boundary = 'left right top bottom front back'
    value = 0
  [../]
[]

[Postprocessors]
  [./l2_norm]
    type = ElementL2Norm
    variable = u
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  [./Quadrature]
    type = GAUSS
    order = FIRST
  [../]
[]

[Outputs]
  csv = true
[]
