sigma = 25e-3 #10e-3 #25e-3 #surface tension coefficient
epsilon = 1e-6 #width parameter
nu = 1e-4#mobility parameter
contactangle = 2.61799#0.523599#1.0472
lambda = ${fparse 3*sigma*epsilon/(2*sqrt(2))}
prefactor_phi = ${fparse nu*lambda/(epsilon*epsilon)}
prefactor_psi = ${fparse -epsilon*epsilon}


[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 0.2e-3
    ymin = 0
    ymax = 0.2e-3
    nx = 20
    ny = 20
    elem_type = QUAD9
  []
[]

[ICs]
  [pf_ic]
    type = BoundingBoxIC
    variable = pf
    x1 = 0.1e-3
    y1 = -0.1e-3
    x2 = 0.3e-03
    y2 = 0.3e-3
    inside = 1
    outside = -1
    int_width = ${fparse 2*sqrt(2)*epsilon}
  []
  [velocity]
    type = VectorConstantIC
    x_value = 0.0
    y_value = 0.0
    variable = velocity
  []
[]


[Variables]
  [pf]
    family = LAGRANGE
    order = second
  []
  [auxpf]
    family = LAGRANGE
    order = second
  []
  [velocity]
    family = LAGRANGE_VEC
  []
[]

[Kernels]

  [velocity_timederivative]
    type = ADVectorTimeDerivative
    variable = velocity
  []

  [phasefield_timederivative]
    type = ADTimeDerivative
    variable = pf
  []

  [phasefield_supg]
    type = ADPhaseFieldTimeDerivativeSUPG
    velocity = velocity
    variable = pf
  []

  [phasefield_advection]
    type = ADPhaseFieldAdvection
    velocity = velocity
    variable = pf
  []

  [phasefield_advection_supg]
    type = ADPhaseFieldAdvectionSUPG
    velocity = velocity
    variable = pf
  []

  [phasefield_laplacian]
    type=ADPrefactorLaplacianSplit
    variable = pf
    c = auxpf
    prefactor = ${prefactor_phi}
  []

  [Auxphasefield_firstorder]
    type=ADReaction
    variable = auxpf
    rate = 1.0
  []

  [Auxphasefield_laplacian]
    type=ADPrefactorLaplacianSplit
    variable = auxpf
    c = pf
    prefactor=${prefactor_psi}
  []

  [Auxphasefield_doublewell]
    type=ADPhaseFieldCoupledDoubleWellPotential
    variable = auxpf
    c = pf
    prefactor=-1.0
  []
[]

[BCs]

  [velocity]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'left right top bottom'
    function_x = 0.0
  []

  [ContactangleBC]
    type=ADPhaseFieldContactAngleBC
    variable = auxpf
    pf = pf
    epsilon = ${epsilon}
    lambda=${lambda}
    sigma=${sigma}
    contactangle=${contactangle}
    boundary = 'top bottom'
  []
[]

[Materials]
  [rho]
    type = ADPhaseFieldTwoPhaseMaterial
    prop_name = rho
    prop_value_1 = 1000
    prop_value_2 = 840
    pf = pf
   # outputs = exodus
  []
  [mu]
    type = ADPhaseFieldTwoPhaseMaterial
    prop_name = mu
    prop_value_1 = 1e-3
    prop_value_2 = 7.6e-3
    pf = pf
   # outputs = exodus
  []

[]

[Postprocessors]
  [contact_angle_top]
    type = ObtainAvgContactAngle
    boundary = top
    pf=pf
    execute_on = 'timestep_end'
  []
  [x_position]
    type = FindValueOnLine
    start_point = '0 0.0001 0'
    end_point ='0.0002 0.0001 0'
    v = pf
    target = 0.0
    tol = 1e-6
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]
[Adaptivity]
  initial_steps = 2
  initial_marker = phase_marker
  marker = phase_marker
  max_h_level = 4
  [Markers]
    [phase_marker]
       type = ValueRangeMarker
       lower_bound = -0.99
       upper_bound = 0.99
      variable = pf
    []
  []
[]
[Executioner]
  type = Transient
  solve_type = NEWTON
  start_time = 0
  # A prescribed sequence keeps the reported times independent of the nonlinear iteration counts,
  # which an adaptive time stepper would otherwise fold into every output time.
  [TimeStepper]
    type = TimeSequenceStepper
    time_sequence = '1e-10 3e-10 7e-10 1.5e-9 3.1e-9'
    use_last_t_for_end_time = true
  []
  # MUMPS is used for its row and column equilibration: the assembled Jacobian entries span many
  # orders of magnitude, and a factorization without equilibration hits a zero pivot here.
  # ICNTL(14) raises the percentage of extra working space MUMPS allocates over its own estimate,
  # which the fill-in of this system exceeds at the default setting.
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -mat_mumps_icntl_14'
  petsc_options_value = 'lu       mumps                      300'
  # A full Newton step here transits a large intermediate residual before descending, so a
  # backtracking line search stalls on the way through.
  line_search = 'none'
  # The interface width of 1e-6 in a 2e-4 domain makes every element integral tiny, which puts the
  # unscaled residual near 1e-12 -- close enough to the roundoff floor of the assembled residual
  # that no absolute tolerance can separate a converged solve from an untouched initial condition.
  # Jacobian-diagonal scaling leaves that magnitude in place, so scale each equation by the
  # magnitude of its own residual instead, which restores an O(1) residual and lets the tolerances
  # below measure convergence rather than the units of the equations.
  automatic_scaling = true
  resid_vs_jac_scaling_param = 1
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 30
  l_tol = 1e-6
  l_max_its = 20
[]

[Outputs]
  [csv]
    type = CSV
    time_step_interval = 1
  []
[]
