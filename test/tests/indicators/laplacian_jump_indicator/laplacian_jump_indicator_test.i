[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 10
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
  type = Steady
  nl_rel_tol = 1.e-11

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
