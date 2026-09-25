# 2D RZ source: constant MONOMIAL tensor components.
# T_rr=2, T_yy=3, T_ry=0.5, T_tt=-1 so that T_rr != T_tt and T_ry != 0,
# making all six Cartesian rotated components distinct.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    xmin = 1
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
    initial_condition = 0.5
  []
  [T_tt]
    family = LAGRANGE
    order = FIRST
    initial_condition = -1.0
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
