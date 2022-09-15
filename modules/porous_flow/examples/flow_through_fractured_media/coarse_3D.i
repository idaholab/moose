# Flow and solute transport along 2 2D eliptical fractures embedded in a 3D porous matrix
# the model domain has dimensions 1 x 1 x 0.3m and the two fracture have r1 = 0.45 and r2 = 0.2
# The fractures intersect each other and the domain boundaries on two opposite sides
# fracture aperture = 6e-4m
# fracture porosity = 6e-4m
# fracture permeability = 1.8e-11 which is based in k=3e-8 from a**2/12, and k*a = 3e-8*6e-4;
# matrix porosity = 0.1;
# matrix permeanility = 1e-20;

[Mesh]
  type = FileMesh
  file = coarse_3D.e
  block_id = '1 2 3'
  block_name = 'matrix f1 f2'

  boundary_id = '1 2 3 4'
  boundary_name = 'rf2 lf1 right_matrix left_matrix'
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [pp]
  []
  [tracer]
  []
[]

[AuxVariables]
  [velocity_x]
    family = MONOMIAL
    order = CONSTANT
    block = 'f1 f2'
  []
  [velocity_y]
    family = MONOMIAL
    order = CONSTANT
    block = 'f1 f2'
  []
  [velocity_z]
    family = MONOMIAL
    order = CONSTANT
    block = 'f1 f2'
  []
[]

[AuxKernels]
  [velocity_x]
    type = PorousFlowDarcyVelocityComponentLowerDimensional
    variable = velocity_x
    component = x
    aperture = 6E-4
  []
  [velocity_y]
    type = PorousFlowDarcyVelocityComponentLowerDimensional
    variable = velocity_y
    component = y
    aperture = 6E-4
  []
  [velocity_z]
    type = PorousFlowDarcyVelocityComponentLowerDimensional
    variable = velocity_z
    component = z
    aperture = 6E-4
  []
[]

[ICs]
  [pp]
    type = ConstantIC
    variable = pp
    value = 1e6
  []
  [tracer]
    type = ConstantIC
    variable = tracer
    value = 0
  []
[]

[BCs]
  [top]
    type = DirichletBC
    value = 0
    variable = tracer
    boundary = rf2
  []
  [bottom]
    type = DirichletBC
    value = 1
    variable = tracer
    boundary = lf1
  []
  [ptop]
    type = DirichletBC
    variable = pp
    boundary =  rf2
    value = 1e6
  []
  [pbottom]
    type = DirichletBC
    variable = pp
    boundary = lf1
    value = 1.02e6
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [adv0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp
  []
  [diff0]
    type = PorousFlowDispersiveFlux
    fluid_component = 0
    variable = pp
    disp_trans = 0
    disp_long = 0
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = tracer
  []
  [adv1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = tracer
  []
  [diff1]
    type = PorousFlowDispersiveFlux
    fluid_component = 1
    variable = tracer
    disp_trans = 0
    disp_long = 0
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp tracer'
    number_fluid_phases = 1
    number_fluid_components = 2
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 1000
    thermal_expansion = 0
    viscosity = 1e-3
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'tracer'
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [poro1]
    type = PorousFlowPorosityConst
    porosity = 6e-4   # = a * phif
    block = 'f1 f2'
  []
  [diff1]
    type = PorousFlowDiffusivityConst
    diffusion_coeff = '1.e-9 1.e-9'
    tortuosity = 1.0
    block = 'f1 f2'
  []
  [poro2]
    type = PorousFlowPorosityConst
    porosity = 0.1
    block = 'matrix'
  []
  [diff2]
    type = PorousFlowDiffusivityConst
    diffusion_coeff = '1.e-9 1.e-9'
    tortuosity = 0.1
    block = 'matrix'
  []
  [relp]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
  []
  [permeability1]
    type = PorousFlowPermeabilityConst
    permeability = '1.8e-11 0 0 0 1.8e-11 0 0 0 1.8e-11'   # 1.8e-11 = a * kf
    block = 'f1 f2'
  []
  [permeability2]
    type = PorousFlowPermeabilityConst
    permeability = '1e-20 0 0 0 1e-20 0 0 0 1e-20'
    block = 'matrix'
  []
[]

[Preconditioning]
  active = basic
  [mumps_is_best_for_parallel_jobs]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
    petsc_options_value = ' lu       mumps'
  []
  [basic]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2             '
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  end_time = 20
  dt = 1
[]

[VectorPostprocessors]
  [xmass]
    type = LineValueSampler
    start_point = '-0.5 0 0'
    end_point = '0.5 0 0'
    sort_by = x
    num_points = 41
    variable = tracer
    outputs = csv
  []
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = 'final'
  []
[]
