[GlobalParams]
  fp = fp
[]

[Mesh]
  [./gen_mesh]
    type = CartesianMeshGenerator
    dim = 1
    dx = '.1 .1 .1 .1 .1 .5 .1 .1 .1 .1 .1'
    # dx = '.1 .1 .1 .1 .1 .1 .1 .1 .1 .1 .1 .1 .1 .1 .1'
  [../]
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Variables]
  [rho]
    type = MooseVariableFVReal
    initial_condition = 1.28969
    scaling = 1e3
  []
  [rho_u]
    type = MooseVariableFVReal
    initial_condition = 1.28969
  []
  [rho_et]
    type = MooseVariableFVReal
    initial_condition = 2.525e5
    scaling = 1e-2
  []
[]

[FVKernels]
  [mass_advection]
    type = CNSFVMassHLLC
    variable = rho
    fp = fp
  []

  [momentum_x_advection]
    type = CNSFVMomentumHLLC
    variable = rho_u
    momentum_component = x
    fp = fp
  []
  [drag]
    type = FVReaction
    variable = rho_u
    rate = 1000
  []

  [fluid_energy_advection]
    type = CNSFVFluidEnergyHLLC
    variable = rho_et
    fp = fp
  []
[]

[FVBCs]
  [mass_in]
    variable = rho
    type = CNSFVHLLCSpecifiedMassFluxAndTemperatureMassBC
    boundary = left
    temperature = 273.15
    rhou = 1.28969
  []
  [momentum_in]
    variable = rho_u
    type = CNSFVHLLCSpecifiedMassFluxAndTemperatureMomentumBC
    boundary = left
    temperature = 273.15
    rhou = 1.28969
    momentum_component = 'x'
  []
  [energy_in]
    variable = rho_et
    type = CNSFVHLLCSpecifiedMassFluxAndTemperatureFluidEnergyBC
    boundary = left
    temperature = 273.15
    rhou = 1.28969
  []

  [mass_out]
    variable = rho
    type = CNSFVHLLCSpecifiedPressureMassBC
    boundary = right
    pressure = 1.01e5
  []
  [momentum_out]
    variable = rho_u
    type = CNSFVHLLCSpecifiedPressureMomentumBC
    boundary = right
    pressure = 1.01e5
    momentum_component = 'x'
  []
  [energy_out]
    variable = rho_et
    type = CNSFVHLLCSpecifiedPressureFluidEnergyBC
    boundary = right
    pressure = 1.01e5
  []
[]

[Materials]
  [var_mat]
    type = ConservedVarValuesMaterial
    rho = rho
    rhou = rho_u
    rho_et = rho_et
  []
[]

[Executioner]
  solve_type = NEWTON
  type = Steady
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_max_its = 50
  line_search = none
[]

[Outputs]
  exodus = true
  csv = true
[]

[Debug]
  show_var_residual_norms = true
[]
