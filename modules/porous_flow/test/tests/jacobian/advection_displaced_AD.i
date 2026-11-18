# The AD advective flux with use_displaced_mesh = true and the displacements nonlinear variables.
# The Darcy flux is weighted by the quadrature weight JxW of the displaced mesh, which depends on
# the displacements, so the Jacobian has a displacement block: PorousFlowDarcyBase must weight by
# _ad_JxW * _ad_coord, as ADKernel does.
# 1phase, 1component, Corey relative permeability, nonzero gravity, unsaturated with vanGenuchten.
# The displacements are held by a kernel on the undisplaced mesh, which makes them variables of the
# system without coupling them to the flow.
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
  []
  displacements = 'disp_x disp_y'
[]

[GlobalParams]
  PorousFlowDictator = dictator
  displacements = 'disp_x disp_y'
  use_displaced_mesh = true
[]

[Variables]
  [pp]
  []
  [disp_x]
  []
  [disp_y]
  []
[]

[ICs]
  [pp]
    type = FunctionIC
    variable = pp
    function = '-0.7+x+y'
  []
  [disp_x]
    type = FunctionIC
    variable = disp_x
    function = '0.1*x*y'
  []
  [disp_y]
    type = FunctionIC
    variable = disp_y
    function = '0.1*x*y'
  []
[]

[Kernels]
  [flux]
    type = ADPorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp
    gravity = '-1 -0.1 0'
  []
  [pp_time]
    type = TimeDerivative
    variable = pp
  []
  [dx]
    type = ADDiffusion
    variable = disp_x
    use_displaced_mesh = false
  []
  [dy]
    type = ADDiffusion
    variable = disp_y
    use_displaced_mesh = false
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
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
  []
  [simple_fluid]
    type = ADPorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [permeability]
    type = ADPorousFlowPermeabilityConst
    permeability = '1 0 0 0 2 0 0 0 3'
  []
  [relperm]
    type = ADPorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
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
  end_time = 1
[]

[Outputs]
  exodus = false
[]
