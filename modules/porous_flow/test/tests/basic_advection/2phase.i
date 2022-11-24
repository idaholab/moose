# Basic advection of u in a 2-phase situation
#
# grad(P) = -2
# density * gravity = 4 * 0.25
# grad(P) - density * gravity = -3
# permeability = 10
# relative permeability = 0.5
# viscosity = 150
# so Darcy velocity = 0.1
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [P0]
  []
  [P1]
  []
[]

[ICs]
  [P0]
    type = FunctionIC
    variable = P0
    function = '0'
  []
  [P1]
    type = FunctionIC
    variable = P1
    function = '2*(1-x)'
  []
  [u]
    type = FunctionIC
    variable = u
    function = 'if(x<0.1,1,0)'
  []
[]

[Kernels]
  [u_dot]
    type = TimeDerivative
    variable = u
  []
  [u_advection]
    type = PorousFlowBasicAdvection
    variable = u
    phase = 1
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = ''
    number_fluid_phases = 2
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
  []
[]

[FluidProperties]
  [simple_fluid0]
    type = SimpleFluidProperties
    density0 = 32
    viscosity = 123
  []
  [simple_fluid1]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 4
    thermal_expansion = 0
    viscosity = 150.0
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow2PhasePP
    phase0_porepressure = P0
    phase1_porepressure = P1
    capillary_pressure = pc
  []
  [simple_fluid0]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid0
    phase = 0
  []
  [simple_fluid1]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid1
    phase = 1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '10 0 0 0 10 0 0 0 10'
  []
  [relperm0]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
  [relperm1]
    type = PorousFlowRelativePermeabilityConst
    kr = 0.5
    phase = 1
  []
  [darcy_velocity]
    type = PorousFlowDarcyVelocityMaterial
    gravity = '0.25 0 0'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    boundary = left
    value = 1
    variable = u
  []
  [right]
    type = DirichletBC
    boundary = right
    value = 0
    variable = u
  []
[]

[Preconditioning]
  [basic]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -snes_rtol'
    petsc_options_value = ' lu       1E-10'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 5
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
