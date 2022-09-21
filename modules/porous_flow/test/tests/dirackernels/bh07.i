# Comparison with analytical solution for cylindrically-symmetric situation
[Mesh]
  type = FileMesh
  file = bh07_input.e
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Functions]
  [dts]
    type = PiecewiseLinear
    y = '1000 10000'
    x = '100 1000'
  []
[]

[Variables]
  [pp]
    initial_condition = 1E7
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [fflux]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp
    gravity = '0 0 0'
  []
[]

[BCs]
  [fix_outer]
    type = DirichletBC
    boundary = perimeter
    variable = pp
    value = 1E7
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [borehole_total_outflow_mass]
    type = PorousFlowSumQuantity
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.8
    alpha = 1e-5
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    viscosity = 1e-3
    density0 = 1000
    thermal_expansion = 0
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-11 0 0 0 1E-11 0 0 0 1E-11'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityFLAC
    m = 2
    phase = 0
  []
[]

[DiracKernels]
  [bh]
    type = PorousFlowPeacemanBorehole
    variable = pp
    SumQuantityUO = borehole_total_outflow_mass
    point_file = bh07.bh
    fluid_phase = 0
    bottom_p_or_t = 0
    unit_weight = '0 0 0'
    use_mobility = true
    re_constant = 0.1594  # use Chen and Zhang version
    character = 2 # double the strength because bh07.bh only fills half the mesh
  []
[]

[Postprocessors]
  [bh_report]
    type = PorousFlowPlotQuantity
    uo = borehole_total_outflow_mass
    execute_on = 'initial timestep_end'
  []
  [fluid_mass]
    type = PorousFlowFluidMass
    execute_on = 'initial timestep_end'
  []
[]

[VectorPostprocessors]
  [pp]
    type = LineValueSampler
    variable = pp
    start_point = '0 0 0'
    end_point = '300 0 0'
    sort_by = x
    num_points = 300
    execute_on = timestep_end
  []
[]

[Preconditioning]
  [usual]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -ksp_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-10 1E-10 10000 30'
  []
[]

[Executioner]
  type = Transient
  end_time = 1E3
  solve_type = NEWTON
  [TimeStepper]
    # get only marginally better results for smaller time steps
    type = FunctionDT
    function = dts
  []
[]

[Outputs]
  file_base = bh07
  [along_line]
    type = CSV
    execute_on = final
  []
  [exodus]
    type = Exodus
    execute_on = 'initial final'
  []
[]
