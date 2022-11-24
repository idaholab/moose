# phase number is too high in PorousFlowBasicAdvection
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
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
  [P]
  []
[]

[ICs]
  [P]
    type = FunctionIC
    variable = P
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
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
  []
[]

[FluidProperties]
  [simple_fluid]
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
    type = PorousFlow1PhaseP
    porepressure = P
    capillary_pressure = pc
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '5 0 0 0 5 0 0 0 5'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 0
    phase = 0
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
