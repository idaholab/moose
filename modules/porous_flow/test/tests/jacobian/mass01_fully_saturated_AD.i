# Jacobian test for ADPorousFlowFullySaturatedMassTimeDerivative.
# The PetscJacobianTester verifies that the AD-computed Jacobian matches
# the finite-difference approximation to machine precision.
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    thermal_expansion = 0.5
    bulk_modulus = 1.5
    density0 = 1.0
  []
[]

[Variables]
  [pp]
  []
[]

[ICs]
  [pp]
    type = RandomIC
    variable = pp
    min = 0
    max = 1
  []
[]

[Kernels]
  [mass0]
    type = ADPorousFlowFullySaturatedMassTimeDerivative
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
[]

[Materials]
  [temperature]
    type = ADPorousFlowTemperature
  []
  [ppss]
    type = ADPorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [massfrac]
    type = ADPorousFlowMassFraction
  []
  [simple_fluid]
    type = ADPorousFlowSingleComponentFluid
    fp = the_simple_fluid
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [biot_modulus]
    type = PorousFlowConstantBiotModulus
    fluid_bulk_modulus = 1.5
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
