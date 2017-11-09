[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  nz = 0
[]

[Variables]
  [./u]
    order = THIRD
    family = HERMITE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./conv]
    type = Convection
    variable = u
    velocity = '1 0 0'
  [../]
  [./time]
    type = TimeDerivative
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
    value = 10
  [../]
  [./divbc]
    type = DivergenceBC
    variable = u
    boundary = 'left right'
  [../]
[]

[Adaptivity]
  [./Indicators]
    [./error]
      type = LaplacianJumpIndicator
      variable = u
      scale_by_flux_faces = true
    [../]
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 4
  dt = 0.1
[]

[Outputs]
  exodus = true
[]
