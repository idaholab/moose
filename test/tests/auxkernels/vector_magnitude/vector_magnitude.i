[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -1
  xmax =  1
  ymin = -1
  ymax =  1
  zmin = -1
  zmax =  1
  nx = 2
  ny = 2
  nz = 2
[../]

[Variables]
  [./u]
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
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[AuxVariables]
  [./vector_x]
    initial_condition = 2
  [../]
  [./vector_y]
    initial_condition = 1
  [../]
  [./vector_z]
    initial_condition = 2
  [../]

  [./magnitude]
  [../]
[]

[AuxKernels]
  [./vx]
    type = ConstantAux
    variable = vector_x
    value = 2
  [../]
  [./vy]
    type = ConstantAux
    variable = vector_y
    value = 1
  [../]
  [./vz]
    type = ConstantAux
    variable = vector_z
    value = 2
  [../]

  [./magnitude]
    type = VectorMagnitudeAux
    variable = magnitude
    x = vector_x
    y = vector_y
    z = vector_z
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
