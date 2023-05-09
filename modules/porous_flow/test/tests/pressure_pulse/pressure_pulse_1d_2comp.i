# Pressure pulse in 1D with 1 phase but 2 components (where density and viscosity depend on mass fraction)
# This test uses BrineFluidProperties with the PorousFlowMultiComponentFluid material, but could be run using
# the PorousFlowBrine material instead.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 100
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp xnacl'
    number_fluid_phases = 1
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
    pc = 0
  []
[]

[Variables]
  [pp]
    initial_condition = 1e6
  []
  [xnacl]
    initial_condition = 0
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = xnacl
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = xnacl
  []
[]

[AuxVariables]
  [density]
    family = MONOMIAL
    order = FIRST
  []
[]

[AuxKernels]
  [density]
    type = PorousFlowPropertyAux
    variable = density
    property = density
    phase = 0
    execute_on = 'initial timestep_end'
  []
[]

[FluidProperties]
  [brine]
    type = BrineFluidProperties
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = 293
  []
  [mass_fractions]
    type = PorousFlowMassFraction
    mass_fraction_vars = xnacl
  []
  [ps]
    type = PorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
  []
  [brine]
    type = PorousFlowMultiComponentFluid
    x = xnacl
    fp = brine
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-7 0 0 0 1e-7 0 0 0 1e-7'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityConst
    kr = 1
    phase = 0
  []
[]

[BCs]
  [left_p]
    type = DirichletBC
    boundary = left
    value = 2e6
    variable = pp
  []
  [right_p]
    type = DirichletBC
    boundary = right
    value = 1e6
    variable = pp
  []
  [left_xnacl]
    type = DirichletBC
    boundary = left
    value = 0.2
    variable = xnacl
  []
  [right_xnacl]
    type = DirichletBC
    boundary = right
    value = 0
    variable = xnacl
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -pc_factor_shift_type'
    petsc_options_value = 'bcgs lu  NONZERO'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 5
[]

[Postprocessors]
  [p000]
    type = PointValue
    variable = pp
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  []
  [p050]
    type = PointValue
    variable = pp
    point = '50 0 0'
    execute_on = 'initial timestep_end'
  []
  [p100]
    type = PointValue
    variable = pp
    point = '100 0 0'
    execute_on = 'initial timestep_end'
  []
  [xnacl_000]
    type = PointValue
    variable = xnacl
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  []
  [density_000]
    type = PointValue
    variable = density
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  []
  [xnacl_020]
    type = PointValue
    variable = xnacl
    point = '20 0 0'
    execute_on = 'initial timestep_end'
  []
  [density_020]
    type = PointValue
    variable = density
    point = '20 0 0'
    execute_on = 'initial timestep_end'
  []
  [xnacl_040]
    type = PointValue
    variable = xnacl
    point = '40 0 0'
    execute_on = 'initial timestep_end'
  []
  [density_040]
    type = PointValue
    variable = density
    point = '40 0 0'
    execute_on = 'initial timestep_end'
  []
  [xnacl_060]
    type = PointValue
    variable = xnacl
    point = '60 0 0'
    execute_on = 'initial timestep_end'
  []
  [density_060]
    type = PointValue
    variable = density
    point = '60 0 0'
    execute_on = 'initial timestep_end'
  []
  [xnacl_080]
    type = PointValue
    variable = xnacl
    point = '80 0 0'
    execute_on = 'initial timestep_end'
  []
  [density_080]
    type = PointValue
    variable = density
    point = '80 0 0'
    execute_on = 'initial timestep_end'
  []
  [xnacl_100]
    type = PointValue
    variable = xnacl
    point = '100 0 0'
    execute_on = 'initial timestep_end'
  []
  [density_100]
    type = PointValue
    variable = density
    point = '100 0 0'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
[]
