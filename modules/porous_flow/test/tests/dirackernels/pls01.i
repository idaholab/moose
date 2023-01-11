# fully-saturated situation with a poly-line sink at one
# of the nodes.  Because there is no fluid flow, the
# other nodes should not experience any change in
# porepressure.
# The poly-line sink has length=2 and weight=0.1, and
# extracts fluid at a constant rate of 1 kg.m^-1.s^-1.
# Therefore, in 1 second it will have extracted a total
# of 0.2 kg.
# The porosity is 0.1, and the elemental volume is 2,
# so the fluid mass at the node in question = 0.2 * density / 4,
# where the 4 is the number of nodes in the element.
# In this simulation density = dens0 * exp(P / bulk), with
# dens0 = 100, and bulk = 20 MPa.
# The initial porepressure P0 = 10 MPa, so the final (after
# 1 second of simulation) is
# P(t=1) = 0.950879 MPa
#
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmin = 0
  xmax = 2
  ymin = 0
  ymax = 1
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
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pls_total_outflow_mass]
    type = PorousFlowSumQuantity
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
    density0 = 100
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
[]

[DiracKernels]
  [pls]
    type = PorousFlowPolyLineSink
    fluid_phase = 0
    point_file = pls01_21.bh
    line_length = 2
    SumQuantityUO = pls_total_outflow_mass
    variable = pp
    p_or_t_vals = '0 1E7'
    fluxes = '1 1'
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

  [p00]
    type = PointValue
    variable = pp
    point = '0 0 0'
    execute_on = timestep_end
  []
  [p01]
    type = PointValue
    variable = pp
    point = '0 1 0'
    execute_on = timestep_end
  []
  [p20]
    type = PointValue
    variable = pp
    point = '2 0 0'
    execute_on = timestep_end
  []
  [p21]
    type = PointValue
    variable = pp
    point = '2 1 0'
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
  file_base = pls01
  exodus = false
  csv = true
  execute_on = timestep_end
[]
