n1 = 1.46

alpha1 =  0.084075389781548382
alpha2 =  0.872433935441263020
beta1 = -0.08086509944859642
beta2 = -0.29284032175563224
eta1 =  -2.6234175064678195
eta2 = 9.9471473598607409

Tb = 300
epsilon = 1.0

nu1 = 2.93e13
kappa = 10000.0

[Functions]
  [T]
    type = ParsedFunction
    expression = '20000*(x^4 - x^2 + 1)'
  []
[]

[Mesh]
  [gmg]
      type = GeneratedMeshGenerator
      dim = 1
      nx = 51
      xmin = -0.5
      xmax = 0.5
  []
[]

[Problem]
  nl_sys_names = 'psi1
                  psi2'

  verbose_setup = true
[]

[Variables]
  [psi1]
    type = MooseVariableFVReal
    solver_sys = 'psi1'
  []

  [psi2]
    type = MooseVariableFVReal
    solver_sys = 'psi2'
  []
[]

[FVKernels]
  [diffusion1]
    type = FVSP3ThermalRadiationDiffusion
    variable = psi1
    epsilon = ${epsilon}
    kappa = ${kappa}
    order = first
  []

  [diffusion2]
    type = FVSP3ThermalRadiationDiffusion
    variable = psi2
    epsilon = ${epsilon}
    kappa = ${kappa}
    order = second
  []

  [sink1]
    type = FVSP3ThermalRadiationSourceSink
    variable = psi1
    T = T
    nu = ${nu1}
    refraction_index = ${n1}
    kappa = ${kappa}
  []

  [sink2]
    type = FVSP3ThermalRadiationSourceSink
    variable = psi2
    T = T
    nu = ${nu1}
    refraction_index = ${n1}
    kappa = ${kappa}
  []
[]

[FVBCs]
  [BC1]
    type = FVSP3ThermalRadiationBC
    boundary = 'left right'
    variable = psi1
    Tb = ${Tb}
    nu = ${nu1}
    refraction_index = ${n1}
    epsilon = ${epsilon}
    psi = 'psi2'
    order = first
    alpha = ${alpha1}
    beta = ${beta2}
    eta = ${eta1}
  []

  [BC2]
    type = FVSP3ThermalRadiationBC
    boundary = 'left right'
    variable = psi2
    Tb = ${Tb}
    nu = ${nu1}
    refraction_index = ${n1}
    epsilon = ${epsilon}
    psi = 'psi1'
    order = second
    alpha = ${alpha2}
    beta = ${beta1}
    eta = ${eta2}
  []
[]

[Executioner]
  type = Steady
  solve_type = 'Newton'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 500'

  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  l_tol = 1e-6

  l_max_its = 2000
  nl_max_its = 500
[]

[VectorPostprocessors]
  [y0]
    num_points = 51
    start_point = '-0.5 0.0 0.0'
    end_point = '0.5 0.0 0.0'
    sort_by = 'x'
    variable = 'psi1 psi2'
    type = LineValueSampler
  []
[]

[Outputs]
  [e]
    type = Exodus
  []
  [csv]
    type = CSV
    execute_on = final
  []
[]
