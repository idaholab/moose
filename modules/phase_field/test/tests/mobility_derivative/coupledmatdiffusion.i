[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 15
  ny = 15
  xmax = 15.0
  ymax = 15.0
  elem_type = QUAD4
[]

[Variables]
  [./c]
    [./InitialCondition]
      type = CrossIC
      x1 = 0.0
      x2 = 30.0
      y1 = 0.0
      y2 = 30.0
    [../]
  [../]
  [./d]
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 15
      y1 = 15
      radius = 8
      int_width = 3
      invalue = 2
      outvalue = 0
    [../]
  [../]
  [./u]
  [../]
  [./w]
  [../]
[]

[Kernels]
  [./ctime]
    type = TimeDerivative
    variable = c
  [../]
  [./umat]
    type = MatReaction
    variable = c
    v = u
    reaction_rate = 1
  [../]
  [./urxn]
    type = Reaction
    variable = u
  [../]
  [./cres]
    type = MatDiffusion
    variable = u
    diffusivity = Dc
    args = d
    v = c
  [../]

  [./dtime]
    type = TimeDerivative
    variable = d
  [../]
  [./wmat]
    type = MatReaction
    variable = d
    v = w
    reaction_rate = 1
  [../]
  [./wrxn]
    type = Reaction
    variable = w
  [../]
  [./dres]
    type = MatDiffusion
    variable = w
    diffusivity = Dd
    args = c
    v = d
  [../]
[]

[Materials]
  [./Dc]
    type = DerivativeParsedMaterial
    property_name = Dc
    expression = '0.01+c^2+d'
    coupled_variables = 'c d'
    derivative_order = 1
  [../]
  [./Dd]
    type = DerivativeParsedMaterial
    property_name = Dd
    expression = 'd^2+c+1.5'
    coupled_variables = 'c d'
    derivative_order = 1
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'BDF2'

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm      31                  lu           1'

  dt = 1
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
