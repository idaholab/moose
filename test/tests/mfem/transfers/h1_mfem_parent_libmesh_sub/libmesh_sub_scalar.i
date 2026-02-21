[Mesh]
  file = ../../mesh/square.e
[]

[Problem]
  type = FEProblem
[]

[Variables]
  [libmesh_scalar_var]
    family = LAGRANGE
    order = FIRST
  []
[]

[BCs]
  [sides]
    type = DirichletBC
    variable = libmesh_scalar_var
    boundary = 'bottom left right top'
    value = 1.0
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = libmesh_scalar_var
  []
  [source]
    type = BodyForce
    variable = libmesh_scalar_var
    value = 2.0
  []  
[]

[Executioner]
  type = Steady
[]
