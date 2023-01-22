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
[]

[UserObjects]
  [pls_total_outflow_mass]
    type = PorousFlowSumQuantity
  []
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1e-7
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e7
    viscosity = 0.2
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
    permeability = '1E-12 0 0 0 1E-12 0 0 0 1E-12'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
[]

[DiracKernels]
  [pls]
    # This defines a sink that has strength
    # f = L(P) * relperm * L_seg
    # where
    #    L(P) is a piecewise-linear function of porepressure
    #      that is zero at pp=0 and 1 at pp=1E7
    #    relperm is the relative permeability of the fluid
    #    L_seg is the line-segment length associated with
    #      the Dirac points defined in the file pls02.bh
    type = PorousFlowPolyLineSink

    # Because the Variable for this Sink is pp, and pp is associated
    # with the fluid-mass conservation equation, this sink is extracting
    # fluid mass (and not heat energy or something else)
    variable = pp

    # The following specfies that the total fluid mass coming out of
    # the porespace via this sink in this timestep should be recorded
    # in the pls_total_outflow_mass UserObject
    SumQuantityUO = pls_total_outflow_mass

    # The following file defines the polyline geometry
    # which is just two points in this particular example
    point_file = pls02.bh

    # Now define the piecewise-linear function, L

    # First, we want L to be a function of porepressure (and not
    # temperature or something else).  The following means that
    # p_or_t_vals should be intepreted by MOOSE as the zeroth-phase
    # porepressure
    function_of = pressure
    fluid_phase = 0

    # Second, define the piecewise-linear function, L
    # The following means
    #    flux=0 when pp=0  (and also pp<0)
    #    flux=1 when pp=1E7  (and also pp>1E7)
    #    flux=linearly intepolated between pp=0 and pp=1E7
    # When flux>0 this means a sink, while flux<0 means a source
    p_or_t_vals = '0 1E7'
    fluxes = '0 1'

    # Finally, in this case we want to always multiply
    # L by the fluid mobility (of the zeroth phase) and
    # use that in the sink strength instead of the bare L
    # computed above
    use_mobility = true
  []
[]

[Postprocessors]
  [pls_report]
    type = PorousFlowPlotQuantity
    uo = pls_total_outflow_mass
  []
  [fluid_mass0]
    type = PorousFlowFluidMass
    execute_on = timestep_begin
  []

  [fluid_mass1]
    type = PorousFlowFluidMass
    execute_on = timestep_end
  []

  [zmass_error]
    type = FunctionValuePostprocessor
    function = mass_bal_fcn
    execute_on = timestep_end
    indirect_dependencies = 'fluid_mass1 fluid_mass0 pls_report'
  []

  [p0]
    type = PointValue
    variable = pp
    point = '0 0 0'
    execute_on = timestep_end
  []
[]

[Functions]
  [mass_bal_fcn]
    type = ParsedFunction
    expression = abs((a-c+d)/2/(a+c))
    symbol_names = 'a c d'
    symbol_values = 'fluid_mass1 fluid_mass0 pls_report'
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
