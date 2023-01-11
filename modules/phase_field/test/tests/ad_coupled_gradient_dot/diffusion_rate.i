# Solves the problem
# -mu * Lap(u_dot) + u_dot = alpha * Lap(u) - 2*u*(1-3*u+2*u^2)
# for mu = 1 and alpha = 0.01
# (see appendix B of A. Guevel et al. "Viscous phase-field modeling for chemo-mechanical microstructural evolution: application to geomaterials and pressure solution." In print.)

n_elem = 100
alpha = 0.01
mu = 1

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = '${n_elem}'
  elem_type = EDGE2
[]

[Variables]
  [u]
    [InitialCondition]
      type = ConstantIC
      value = 0.51
    []
  []
[]

[Kernels]
  [Lap]
    type = ADMatDiffusion
    variable = u
    diffusivity = '${alpha}'
  []
  [LapDot]
    type = ADDiffusionRate
    variable = u
    mu = '${mu}'
  []
  [Reac]
    type = ADMatReaction
    variable = u
    reaction_rate = L
  []
  [Visc]
    type = ADTimeDerivative
    variable = u
  []
[]

[Materials]
  [parsed]
    type = ADParsedMaterial
    expression = '-2*(1-3*u+2*u*u)'
    coupled_variables = 'u'
    property_name = 'L'
  []
[]

[BCs]
  [both]
    type = ADDirichletBC
    variable = u
    value = 0.51
    boundary = 'left right'
  []
[]

[Postprocessors]
  [mid_u]
    type = PointValue
    variable = u
    point = '0.5 0 0'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  num_steps = 1000
  dt = 0.1

  nl_abs_tol = 1e-9

[]

[Outputs]
  print_linear_residuals = false
  [csv]
    type = CSV
    file_base = 'diffusion_rate'
  []
[]
