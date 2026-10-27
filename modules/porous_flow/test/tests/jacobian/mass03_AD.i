# Jacobian test for ADPorousFlowMassTimeDerivative
# 1phase, vanGenuchten, constant-bulk density, constant porosity, 3components, unsaturated
# AD path: Jacobian verified against finite differences via PetscJacobianTester
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
  []
  [mass_frac_comp0]
  []
  [mass_frac_comp1]
  []
[]

[ICs]
  [pp]
    type = RandomIC
    variable = pp
    min = -1
    max = 0
  []
  [mass_frac_comp0]
    type = RandomIC
    variable = mass_frac_comp0
    min = 0
    max = 0.3
  []
  [mass_frac_comp1]
    type = RandomIC
    variable = mass_frac_comp1
    min = 0
    max = 0.3
  []
[]

[Kernels]
  [mass_comp0]
    type = ADPorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [masscomp1]
    type = ADPorousFlowMassTimeDerivative
    fluid_component = 1
    variable = mass_frac_comp0
  []
  [masscomp2]
    type = ADPorousFlowMassTimeDerivative
    fluid_component = 2
    variable = mass_frac_comp1
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp mass_frac_comp0 mass_frac_comp1'
    number_fluid_phases = 1
    number_fluid_components = 3
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
    s_scale = 0.9
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1.5
    density0 = 1
    thermal_expansion = 0
    viscosity = 1
  []
[]

[Materials]
  [temperature]
    type = ADPorousFlowTemperature
  []
  [ppss]
    type = ADPorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
  []
  [massfrac]
    type = ADPorousFlowMassFraction
    mass_fraction_vars = 'mass_frac_comp0 mass_frac_comp1'
  []
  [simple_fluid]
    type = ADPorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = ADPorousFlowPorosityConst
    porosity = 0.1
  []
[]

[Preconditioning]
  [check]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 2
[]

[Outputs]
  exodus = false
[]
