# checking that the mass postprocessor correctly calculates the mass
# 1phase, 2component, constant porosity
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 3
  xmin = -1
  xmax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
  []
  [mass_frac_comp0]
  []
[]

[ICs]
  [pinit]
    type = FunctionIC
    function = x
    variable = pp
  []
  [minit]
    type = FunctionIC
    function = 'x*x'
    variable = mass_frac_comp0
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = mass_frac_comp0
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp mass_frac_comp0'
    number_fluid_phases = 1
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1
    density0 = 1
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
    mass_fraction_vars = 'mass_frac_comp0'
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

[Postprocessors]
  [total_mass_0]
    type = PorousFlowFluidMass
  []
  [total_mass_1]
    type = PorousFlowFluidMass
    fluid_component = 1
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1 1 10000'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = mass02
  csv = true
[]
