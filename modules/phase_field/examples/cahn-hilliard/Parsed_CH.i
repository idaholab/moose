#
# Example problem showing how to use the DerivativeParsedMaterial with CHParsed.
# The free energy is identical to that from CHMath, f_bulk = 1/4*(1-c)^2*(1+c)^2.
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
  xmax = 60
  ymax = 60
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = THIRD
    family = HERMITE
  [../]
[]

[ICs]
  [./cIC]
    type = RandomIC
    variable = c
    max = 0.1
    min = -0.1
  [../]
[]

[Kernels]
active = 'c_dot CH_Parsed CHint'
  [./c_dot]
    type = TimeDerivative
    variable = c
  [../]
  [./CH_Parsed]
    type = CHParsed
    variable = c
    f_name = fbulk
    mob_name = M
  [../]
  [./CH_Math]
    type = CHMath
    variable = c
  [../]
  [./CHint]
    type = CHInterface
    variable = c
    mob_name = M
    kappa_name = kappa_c
    grad_mob_name = grad_M
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./mat]
    type = PFMobility
    block = 0
    mob = 1.0
    kappa = 0.5
  [../]

  [./free_energy]
    type = DerivativeParsedMaterial
    block = 0
    f_name = fbulk
    args = 'c'
    constant_names = 'W'
    constant_expressions = '1.0/2^2'
    function = 'W*(1-c)^2*(1+c)^2'
    enable_jit = true
  [../]
[]

[Postprocessors]
  [./top]
    type = SideIntegralVariablePostprocessor
    variable = c
    boundary = top
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  dt = 2.0

  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'

  l_max_its = 30
  l_tol = 1e-4
  nl_max_its = 20
  nl_rel_tol = 1e-9
  end_time = 20.0
[]

[Outputs]
  output_initial = true
  exodus = true
  active = 'console'
  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
  [../]
[]
