# Using a single-dimensional mesh
# Transient flow and solute transport along a fracture in a porous matrix
# advective dominated flow in the fracture and diffusion into the porous matrix
#
# Note that fine_thick_fracture_steady.i must be run to initialise the porepressure properly

[Mesh]
  file = 'gold/fine_thick_fracture_steady_out.e'
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [pp]
    initial_from_file_var = pp
    initial_from_file_timestep = 1
  []
  [massfrac0]
  []
[]

[AuxVariables]
  [velocity_x]
    family = MONOMIAL
    order = CONSTANT
    block = fracture
  []
  [velocity_y]
    family = MONOMIAL
    order = CONSTANT
    block = fracture
  []
[]

[AuxKernels]
  [velocity_x]
    type = PorousFlowDarcyVelocityComponent
    variable = velocity_x
    component = x
  []
  [velocity_y]
    type = PorousFlowDarcyVelocityComponent
    variable = velocity_y
    component = y
  []
[]

[ICs]
  [massfrac0]
    type = ConstantIC
    variable = massfrac0
    value = 0
  []
[]

[BCs]
  [top]
    type = DirichletBC
    value = 0
    variable = massfrac0
    boundary = top
  []
  [bottom]
    type = DirichletBC
    value = 1
    variable = massfrac0
    boundary = bottom
  []
  [ptop]
    type = DirichletBC
    variable = pp
    boundary =  top
    value = 1e6
  []
  [pbottom]
    type = DirichletBC
    variable = pp
    boundary = bottom
    value = 1.002e6
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
    variable = massfrac0
  []
  [adv1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = massfrac0
  []
  [diff1]
    type = PorousFlowDispersiveFlux
    fluid_component = 1
    variable = massfrac0
    disp_trans = 0
    disp_long = 0
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp massfrac0'
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
    mass_fraction_vars = massfrac0
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [poro_fracture]
    type = PorousFlowPorosityConst
    porosity = 1.0    # this is the true porosity of the fracture
    block = 'fracture'
  []
  [poro_matrix]
    type = PorousFlowPorosityConst
    porosity = 0.1
    block = 'matrix1 matrix2'
  []
  [diff1]
    type = PorousFlowDiffusivityConst
    diffusion_coeff = '1e-9 1e-9'
    tortuosity = 1.0
    block = 'fracture'
  []
  [diff2]
    type = PorousFlowDiffusivityConst
    diffusion_coeff = '1e-9 1e-9'
    tortuosity = 0.1
    block = 'matrix1 matrix2'
  []
  [relp]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
  []
  [permeability1]
    type = PorousFlowPermeabilityConst
    permeability = '3e-8 0 0 0 3e-8 0 0 0 3e-8' # this is the true permeability of the fracture
    block = 'fracture'
  []
  [permeability2]
    type = PorousFlowPermeabilityConst
    permeability = '1e-20 0 0 0 1e-20 0 0 0 1e-20'
    block = 'matrix1 matrix2'
  []
[]

[Functions]
  [dt_controller]
     type = PiecewiseConstant
     x = '0    30   40 100 200 83200'
     y = '0.01 0.1  1  10  100 32'
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
  end_time = 86400
  #dt = 0.01

  [TimeStepper]
    type = FunctionDT
    function = dt_controller
  []

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-14
  nl_abs_tol = 1e-9
[]


[VectorPostprocessors]
  [xmass]
    type = LineValueSampler
    start_point = '0.4 0 0'
    end_point = '0.5 0 0'
    sort_by = x
    num_points = 167
    variable = massfrac0
  []
[]

[Outputs]
  perf_graph = true
  console = true
  csv = true
  exodus = true
[]
