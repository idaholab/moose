[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  uniform_refine = 1
[]

[Variables]
  active = 'diffusing_advected diffusing'

  [./diffusing_advected]
    order = FIRST
    family = LAGRANGE
  [../]

  [./diffusing]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diffuse_diffusing_advected advect_diffusing_advected diffuse_diffusing'

  [./diffuse_diffusing_advected]
    type = FDDiffusion
    variable = diffusing_advected
  [../]

  [./advect_diffusing_advected]
    type = FDAdvection
    variable = diffusing_advected
    advector = diffusing
  [../]

  [./diffuse_diffusing]
    type = FDDiffusion
    variable = diffusing
  [../]
[]

[BCs]
  active = 'bottom_diffusing_advected top_diffusing_advected bottom_diffusing top_diffusing'

  [./bottom_diffusing_advected]
    type = DirichletBC
    variable = diffusing_advected
    boundary = 'bottom'
    value = 1
  [../]

  [./top_diffusing_advected]
    type = DirichletBC
    variable = diffusing_advected
    boundary = 'top'
    value = 0
  [../]

  [./bottom_diffusing]
    type = DirichletBC
    variable = diffusing
    boundary = 'bottom'
    value = 2
  [../]

  [./top_diffusing]
    type = DirichletBC
    variable = diffusing
    boundary = 'top'
    value = 0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options = '-snes_view'
  petsc_options_iname = '-ksp_type -ksp_pc_side -pc_type'
  petsc_options_value = '    gmres         left       lu'
[]

[Output]
  linear_residuals = true
  interval = 1
  exodus = true
  perf_log = true
[]


