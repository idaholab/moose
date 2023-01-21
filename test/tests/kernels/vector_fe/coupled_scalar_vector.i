[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmin = -1
  ymin = -1
  elem_type = QUAD9
[]

[Variables]
  [./u]
    family = NEDELEC_ONE
    order = FIRST
  [../]
  [./v]
  [../]
[]

[Kernels]
  [./wave]
    type = VectorFEWave
    variable = u
    x_forcing_func = 'x_ffn'
    y_forcing_func = 'y_ffn'
  [../]
  [./diff]
    type = Diffusion
    variable = v
  [../]
  [./source]
    type = BodyForce
    variable = v
  [../]
  [./advection]
    type = EFieldAdvection
    variable = v
    efield = u
    charge = 'positive'
    mobility = 100
  [../]
[]

[BCs]
  [./bnd]
    type = VectorCurlPenaltyDirichletBC
    boundary = 'left right top bottom'
    penalty = 1e10
    function_x = 'x_sln'
    function_y = 'y_sln'
    variable = u
  [../]
  [./bnd_v]
    type = DirichletBC
    boundary = 'left right top bottom'
    value = 0
    variable = v
  [../]
[]

[Functions]
  [./x_ffn]
    type = ParsedFunction
    expression = '(2*pi*pi + 1)*cos(pi*x)*sin(pi*y)'
  [../]
  [./y_ffn]
    type = ParsedFunction
    expression = '-(2*pi*pi + 1)*sin(pi*x)*cos(pi*y)'
  [../]
  [./x_sln]
    type = ParsedFunction
    expression = 'cos(pi*x)*sin(pi*y)'
  [../]
  [./y_sln]
    type = ParsedFunction
    expression = '-sin(pi*x)*cos(pi*y)'
  [../]
[]

[Preconditioning]
  [./pre]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'asm'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_linesearch_monitor'
[]

[Outputs]
  exodus = true
[]
