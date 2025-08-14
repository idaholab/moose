## This test verifies the implmentation of the PhaseFieldContactAngle , ObtainAvgContactAngle and PhaseFieldTwoPhaseSurfaceTension kernels.

sigma = 25e-3 #10e-3 #25e-3 #surface tension coefficient
epsilon = 1e-6 #width parameter
nu = 1e-4#mobility parameter

##if contact angle is 30 degree, in this model you have to specify (180-contact angle)*pi/180
contactangle = 2.61799
lambda = ${fparse 3*sigma*epsilon/(2*sqrt(2))} 
prefactor_phi = ${fparse nu*lambda/(epsilon*epsilon)}
prefactor_psi = ${fparse -epsilon*epsilon}
coeff = ${fparse lambda/(epsilon*epsilon)}

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
  [p]
  []
[]

[Kernels]
  [mass]
    type = INSADMass
    variable = p
  []
  [mass_pspg]
    type = INSADMassPSPG
    variable = p
  []

  [momentum_viscous]
    type = INSADMomentumViscous
    variable = velocity
   viscous_form = 'traction'
    mu_name = 'mu'
  []

  [momentum_pressure]
    type = INSADMomentumPressure
    variable = velocity
    pressure = p
    integrate_p_by_parts = false
  []

  [surface_tension]
    type = PhaseFieldTwoPhaseSurfaceTension
    variable = velocity
    pf = pf
    auxpf = auxpf
    coeff = ${coeff}
  []

  [phasefield_timederivative]
    type = ADTimeDerivative
    variable = pf
  []

  [phasefield_supg]
    type = PhaseFieldTimeDerivativeSUPG
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
    type=PhaseFieldCoupledDoubleWellPotential
    variable = auxpf
    c = pf
    prefactor=-1.0
  []
[]

[BCs]
  [no_slip]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'top bottom'
  []
  [velocity_L]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'left'
  []

  [velocity_R]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'right'
  []

  [ContactangleBC]
    type=PhaseFieldContactAngleBC
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
    type = PhaseFieldTwoPhaseMaterial
    prop_name = rho
    prop_value_1 = 1000
    prop_value_2 = 840
    pf = pf
   # outputs = exodus
  []
  [mu]
    type = PhaseFieldTwoPhaseMaterial
    prop_name = mu
    prop_value_1 = 1e-3
    prop_value_2 = 7.6e-3   
    pf = pf
   # outputs = exodus
  []

  [ins_mat]
    type = INSADTauMaterial
    velocity = velocity
    pressure = p
    alpha = .1
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
  end_time = 4e-8
  dtmax = 0.25
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-10
    iteration_window = 2
    optimal_iterations = 10
    growth_factor = 2
    cutback_factor = 0.5
  []
  # petsc_options_iname = '-pc_type -ksp_gmres_restart -pc_factor_mat_solver_type -pc_factor_shift_type -pc_factor_shift_amount'
  # petsc_options_value = 'lu       50                  superlu_dist              NONZERO               1e-15'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu      '
  line_search = 'none'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8
  nl_max_its = 20
  nl_forced_its = 3
  l_tol = 1e-6
  l_max_its = 20
[]

[Outputs]
  [csv]
    type = CSV
    time_step_interval = 1  
  []
[]
