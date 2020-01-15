# This model tests the MaxIncrement damper. The converged solution field
# u varies from 0 to 1 across the domain due to the BCs applied. A value
# for the maximum allowed increment to the solution vector for each
# NL iteration is specified. The more restrictive this value is, the
# larger the number of NL iterations will be. This test ensures that a
# minimum number of NL iterations are taken under those conditions.

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
  elem_type = QUAD9
[]

[Variables]
  [./u]
    order = SECOND
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    preset = false
    boundary = left
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    preset = false
    boundary = right
    value = 1
  [../]
[]

[Dampers]
  [./max_inc_damp]
    type = MaxIncrement
    max_increment = 0.1
    variable = u
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]
