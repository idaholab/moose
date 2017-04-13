# Comparison with analytical solution for cylindrically-symmetric situation
[Mesh]
  type = FileMesh
  file = bh07_input.e
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [./pp]
    initial_condition = 1E7
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  [../]
  [./fflux]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp
    gravity = '0 0 0'
  [../]
[]

[BCs]
  [./fix_outer]
    type = DirichletBC
    boundary = perimeter
    variable = pp
    value = 1E7
  [../]
[]


[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
  [./borehole_total_outflow_mass]
    type = PorousFlowSumQuantity
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    at_nodes = true
  [../]
  [./temperature_qp]
    type = PorousFlowTemperature
  [../]
  [./ppss_nodal]
    type = PorousFlow1PhaseP_VG
    at_nodes = true
    porepressure = pp
    al = 1E-5
    m = 0.8
  [../]
  [./ppss_qp]
    type = PorousFlow1PhaseP_VG
    porepressure = pp
    al = 1E-5
    m = 0.8
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1000
    bulk_modulus = 2E9
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
    include_old = true
  [../]
  [./dens0_qp]
    type = PorousFlowDensityConstBulk
    density_P0 = 1000
    bulk_modulus = 2E9
    phase = 0
  [../]
  [./dens_all_qp]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    at_nodes = true
    porosity = 0.1
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-11 0 0 0 1E-11 0 0 0 1E-11'
  [../]
  [./relperm]
    type = PorousFlowRelativePermeabilityFLAC
    at_nodes = true
    m = 2
    phase = 0
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_relative_permeability_nodal
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    at_nodes = true
    viscosity = 1E-3
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_viscosity_nodal
  [../]
[]


[DiracKernels]
  [./bh]
    type = PorousFlowPeacemanBorehole
    bottom_p_or_t = 0
    fluid_phase = 0
    point_file = bh07.bh
    use_mobility = true
    SumQuantityUO = borehole_total_outflow_mass
    variable = pp
    unit_weight = '0 0 0'
    re_constant = 0.1594
    character = 2
  [../]
[]


[Postprocessors]
  [./bh_report]
    type = PorousFlowPlotQuantity
    uo = borehole_total_outflow_mass
    execute_on = 'initial timestep_end'
  [../]
  [./fluid_mass]
    type = PorousFlowFluidMass
    execute_on = 'initial timestep_end'
  [../]
[]

[VectorPostprocessors]
  [./pp]
    type = LineValueSampler
    variable = pp
    start_point = '0 0 0'
    end_point = '300 0 0'
    sort_by = x
    num_points = 300
    execute_on = timestep_end
  [../]
[]

[Preconditioning]
  [./usual]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -ksp_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-10 1E-10 10000 30'
  [../]
[]


[Executioner]
  type = Transient
  end_time = 1E3
  solve_type = NEWTON
  [./TimeStepper]
    # get only marginally better results for smaller time steps
    type = FunctionDT
    time_dt = '1000 10000'
    time_t = '100 1000'
  [../]
[]

[Outputs]
  file_base = bh07
  exodus = true
  interval = 10000
  execute_on = 'initial timestep_end final'
  [./along_line]
    type = CSV
    execute_on = final
  [../]
[]
