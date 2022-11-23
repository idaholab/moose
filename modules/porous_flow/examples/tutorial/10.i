# Unsaturated Darcy-Richards flow without using an Action
[Mesh]
  [annular]
    type = AnnularMeshGenerator
    nr = 10
    rmin = 1.0
    rmax = 10
    growth_r = 1.4
    nt = 4
    dmin = 0
    dmax = 90
  []
  [make3D]
    input = annular
    type = MeshExtruderGenerator
    extrusion_vector = '0 0 12'
    num_layers = 3
    bottom_sideset = 'bottom'
    top_sideset = 'top'
  []
  [shift_down]
    type = TransformGenerator
    transform = TRANSLATE
    vector_value = '0 0 -6'
    input = make3D
  []
  [aquifer]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 0 -2'
    top_right = '10 10 2'
    input = shift_down
  []
  [injection_area]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'x*x+y*y<1.01'
    included_subdomain_ids = 1
    new_sideset_name = 'injection_area'
    input = 'aquifer'
  []
  [rename]
    type = RenameBlockGenerator
    old_block = '0 1'
    new_block = 'caps aquifer'
    input = 'injection_area'
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = pp
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    alpha = 1E-6
    m = 0.6
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
  []
[]

[Kernels]
  [time_derivative]
    type = PorousFlowMassTimeDerivative
    variable = pp
  []
  [flux]
    type = PorousFlowAdvectiveFlux
    variable = pp
    gravity = '0 0 0'
  []
[]

[AuxVariables]
  [sat]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [saturation]
    type = PorousFlowPropertyAux
    variable = sat
    property = saturation
  []
[]


[BCs]
  [production]
    type = PorousFlowSink
    variable = pp
    fluid_phase = 0
    flux_function = 1E-2
    use_relperm = true
    boundary = injection_area
  []
[]

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2E9
    viscosity = 1.0E-3
    density0 = 1000.0
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosity
    porosity_zero = 0.1
  []
  [permeability_aquifer]
    type = PorousFlowPermeabilityConst
    block = aquifer
    permeability = '1E-14 0 0   0 1E-14 0   0 0 1E-14'
  []
  [permeability_caps]
    type = PorousFlowPermeabilityConst
    block = caps
    permeability = '1E-15 0 0   0 1E-15 0   0 0 1E-16'
  []
  [saturation_calculator]
    type = PorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
  []
  [temperature]
    type = PorousFlowTemperature
    temperature = 293
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = the_simple_fluid
    phase = 0
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 3
    s_res = 0.1
    sum_s_res = 0.1
    phase = 0
  []
[]

[Preconditioning]
  active = basic
  [basic]
    type = SMP
    full = true
    petsc_options = '-ksp_diagonal_scale -ksp_diagonal_scale_fix'
    petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = ' asm      lu           NONZERO                   2'
  []
  [preferred_but_might_not_be_installed]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
    petsc_options_value = ' lu       mumps'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1E6
  dt = 1E5
  nl_abs_tol = 1E-7
[]

[Outputs]
  exodus = true
[]
