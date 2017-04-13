# fully-saturated situation with a poly-line sink with use_mobility=true
# The poly-line consists of 2 points, and has a length
# of 0.5.  Each point is weighted with a weight of 0.1
# The PorousFlowPolyLineSink has
# p_or_t_vals = 0 1E7
# fluxes = 0 1
# so that for 0<=porepressure<=1E7
# base flux = porepressure * 1E-6 * mobility  (measured in kg.m^-1.s^-1),
# and when multiplied by the poly-line length, and
# the weighting of each point, the mass flux is
# flux = porepressure * 0.5*E-8 * mobility (kg.s^-1).
#
# The fluid and matrix properties are:
# porosity = 0.1
# element volume = 8 m^3
# density = dens0 * exp(P / bulk), with bulk = 2E7
# initial porepressure P0 = 1E7
# viscosity = 0.2
# So, fluid mass = 0.8 * density (kg)
#
# The equation to solve is
# d(Mass)/dt = - porepressure * 0.5*E-8 * density / viscosity
#
# PorousFlow discretises time to conserve mass, so to march
# forward in time, we must solve
# Mass(dt) = Mass(0) - P * 0.5E-8 * density / viscosity * dt
# or
# 0.8 * dens0 * exp(P/bulk) = 0.8 * dens0 * exp(P0/bulk) - P * 0.5E-8 * density / viscosity * dt
# For the numbers written above this gives
# P(t=1) = 6.36947 MPa
# which is given precisely by MOOSE
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  zmin = -1
  zmax = 1
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
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
  [./pls_total_outflow_mass]
    type = PorousFlowSumQuantity
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    at_nodes = true
  [../]
  [./ppss_nodal]
    type = PorousFlow1PhaseP_VG
    at_nodes = true
    porepressure = pp
    al = 1E-7
    m = 0.5
  [../]
  [./ppss_qp]
    type = PorousFlow1PhaseP_VG
    porepressure = pp
    al = 1E-7
    m = 0.5
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1000
    bulk_modulus = 2E7
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
    include_old = true
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    at_nodes = true
    porosity = 0.1
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-12 0 0 0 1E-12 0 0 0 1E-12'
  [../]
  [./relperm]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 2
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
    viscosity = 0.2
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_viscosity_nodal
  [../]
[]


[DiracKernels]
  [./pls]
    type = PorousFlowPolyLineSink
    fluid_phase = 0
    point_file = pls02.bh
    use_mobility = true
    SumQuantityUO = pls_total_outflow_mass
    variable = pp
    p_or_t_vals = '0 1E7'
    fluxes = '0 1'
  [../]
[]


[Postprocessors]
  [./pls_report]
    type = PorousFlowPlotQuantity
    uo = pls_total_outflow_mass
  [../]

  [./fluid_mass0]
    type = PorousFlowFluidMass
    execute_on = timestep_begin
  [../]

  [./fluid_mass1]
    type = PorousFlowFluidMass
    execute_on = timestep_end
  [../]

  [./zmass_error]
    type = FunctionValuePostprocessor
    function = mass_bal_fcn
    execute_on = timestep_end
  [../]

  [./p0]
    type = PointValue
    variable = pp
    point = '0 0 0'
    execute_on = timestep_end
  [../]
[]


[Functions]
  [./mass_bal_fcn]
    type = ParsedFunction
    value = abs((a-c+d)/2/(a+c))
    vars = 'a c d'
    vals = 'fluid_mass1 fluid_mass0 pls_report'
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
  end_time = 1
  dt = 1
  solve_type = NEWTON
[]

[Outputs]
  file_base = pls02
  exodus = false
  csv = true
  execute_on = timestep_end
[]
