# 1phase, 1component, fully saturated, nonzero gravity
# Density and viscosity from TabulatedFVFFluidProperties, so the Jacobian
# includes the pressure derivatives of the interpolated 1/B and 1/(B mu).
# The porepressure range (-0.7 to 1.3) spans the table and both extrapolated ends.
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  xmin = 0
  xmax = 1
  ny = 1
  ymin = 0
  ymax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
  []
[]

[ICs]
  [pp]
    type = FunctionIC
    variable = pp
    function = -0.7+x+y
  []
[]

[Kernels]
  [flux0]
    type = PorousFlowFullySaturatedDarcyFlow
    fluid_component = 0
    variable = pp
    gravity = '-1 -0.1 0'
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

[FluidProperties]
  [tabulated]
    type = TabulatedFVFFluidProperties
    density_sc = 1
    pressure = '0 0.5 1'
    formation_volume_factor = '1.2 1.1 1.05'
    viscosity = '1 0.9 0.8'
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [fluid]
    type = PorousFlowSingleComponentFluid
    fp = tabulated
    phase = 0
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0 0 2 0 0 0 3'
  []
[]

[Preconditioning]
  active = check
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
