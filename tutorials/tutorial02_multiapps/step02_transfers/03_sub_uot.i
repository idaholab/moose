[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 10

  xmin = -0.05
  xmax = 0.05
  ymin = -0.05
  ymax = 0.05
  zmax = 3
[]

[Variables]
  [v]
  []
[]

[AuxVariables]
  [u_integral]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = v
  []
  [td]
    type = TimeDerivative
    variable = v
  []
[]

[BCs]
  [front]
    type = DirichletBC
    variable = v
    boundary = front
    value = 0
  []
  [back]
    type = DirichletBC
    variable = v
    boundary = back
    value = 1
  []
[]

[Executioner]
  type = Transient
  end_time = 2
  dt = 0.2

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[UserObjects]
  [layered_average]
    type = NearestPointLayeredAverage
    points = '0 0 0'
    direction = z
    num_layers = 4
    variable = v
  []
[]
