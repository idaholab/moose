[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmin = 0
  xmax = 50
  ymin = 0
  ymax = 50
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./c]
     type = LatticeSmoothCircleIC
     variable = c
     invalue = 1.0
     outvalue = 0.0001
     circles_per_side = '2 2'
     pos_variation = 10.0
     radius = 8.0
     int_width = 5.0
     radius_variation_type = uniform
     avoid_bounds = false
  [../]
[]

[BCs]
  [./Periodic]
    [./c]
      variable = c
      auto_direction = 'x y'
    [../]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = c
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
