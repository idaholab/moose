# 2D RZ source for on-axis legality test.
# T_rr = T_tt = 2, T_ry = 0: satisfies the on-axis guard.
# T_yy = 3 to keep the tensor non-trivial.
# xmin=0 so the left boundary touches the axis.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 6
    xmin = 0
    xmax = 6
    ny = 4
    ymin = 0
    ymax = 4
  []
  coord_type = RZ
  rz_coord_axis = y
[]

[Variables]
  [u]
    family = LAGRANGE
    order = FIRST
    initial_condition = 0
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  []
[]

[AuxVariables]
  [T_rr]
    family = LAGRANGE
    order = FIRST
    initial_condition = 2.0
  []
  [T_yy]
    family = LAGRANGE
    order = FIRST
    initial_condition = 3.0
  []
  [T_ry]
    family = LAGRANGE
    order = FIRST
    initial_condition = 0.0
  []
  [T_tt]
    family = LAGRANGE
    order = FIRST
    initial_condition = 2.0
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
