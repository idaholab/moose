inlet_vel = 120
rho_in = 0.8719748696
H_in = 4.0138771448e+05
gamma = 1.4
R = 8.3145
molar_mass = 29e-3
R_specific = ${fparse R / molar_mass}
cp = ${fparse gamma * R_specific / (gamma - 1)}
cv = ${fparse cp / gamma}
T_in = ${fparse H_in / gamma / cv}

mass_flux = ${fparse inlet_vel * rho_in}

outlet_pressure = 0.9e5

[GlobalParams]
  fp = fp
[]

[Debug]
   show_material_props = true
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = subsonic_nozzle.e
  []
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Variables]
  [rho]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 0.8719748696
  []

  [rho_u]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1e-4
  []

  [rho_v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []

  [rho_E]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 2.5e5
  []
[]

[FVKernels]
  # Mass conservation
  [mass_time]
    type = FVTimeKernel
    variable = rho
  []

  [mass_advection]
    type = CNSFVMassHLLC
    variable = rho
  []

  # Momentum x conservation
  [momentum_x_time]
    type = FVTimeKernel
    variable = rho_u
  []

  [momentum_x_advection]
    type = CNSFVMomentumHLLC
    variable = rho_u
    momentum_component = x
  []

  # Momentum y conservation
  [momentum_y_time]
    type = FVTimeKernel
    variable = rho_v
  []

  [momentum_y_advection]
    type = CNSFVMomentumHLLC
    variable = rho_v
    momentum_component = y
  []

  # Fluid energy conservation
  [fluid_energy_time]
    type = FVTimeKernel
    variable = rho_E
  []

  [fluid_energy_advection]
    type = CNSFVFluidEnergyHLLC
    variable = rho_E
  []
[]

[FVBCs]
  ## inflow boundaries
  [mass_inflow]
    type = CNSFVHLLCSpecifiedMassFluxAndTemperatureMassBC
    variable = rho
    boundary = left
    rhou = ${mass_flux}
    rhov = 0
    temperature = ${T_in}
  []

  [momentum_x_inflow]
    type = CNSFVHLLCSpecifiedMassFluxAndTemperatureMomentumBC
    variable = rho_u
    boundary = left
    rhou = ${mass_flux}
    rhov = 0
    temperature = ${T_in}
    momentum_component = x
  []

  [momentum_y_inflow]
    type = CNSFVHLLCSpecifiedMassFluxAndTemperatureMomentumBC
    variable = rho_v
    boundary = left
    rhou = ${mass_flux}
    rhov = 0
    temperature = ${T_in}
    momentum_component = y
  []

  [fluid_energy_inflow]
    type = CNSFVHLLCSpecifiedMassFluxAndTemperatureFluidEnergyBC
    variable = rho_E
    boundary = left
    rhou = ${mass_flux}
    rhov = 0
    temperature = ${T_in}
  []

  ## outflow conditions
  [mass_outflow]
    type = CNSFVHLLCSpecifiedPressureMassBC
    variable = rho
    boundary = right
    pressure = ${outlet_pressure}
  []

  [momentum_x_outflow]
    type = CNSFVHLLCSpecifiedPressureMomentumBC
    variable = rho_u
    boundary = right
    momentum_component = x
    pressure = ${outlet_pressure}
  []

  [momentum_y_outflow]
    type = CNSFVHLLCSpecifiedPressureMomentumBC
    variable = rho_v
    boundary = right
    momentum_component = y
    pressure = ${outlet_pressure}
  []

  [fluid_energy_outflow]
    type = CNSFVHLLCSpecifiedPressureFluidEnergyBC
    variable = rho_E
    boundary = right
    pressure = ${outlet_pressure}
    []

  # wall conditions
  [momentum_x_pressure_wall]
    type = CNSFVMomImplicitPressureBC
    variable = rho_u
    momentum_component = x
    boundary = wall
  []

  [momentum_y_pressure_wall]
    type = CNSFVMomImplicitPressureBC
    variable = rho_v
    momentum_component = y
    boundary = wall
  []
[]

[AuxVariables]
  [Ma]
    family = MONOMIAL
    order = CONSTANT
  []

  [p]
    family = MONOMIAL
    order = CONSTANT
  []

  [Ma_layered]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[UserObjects]
  [layered_Ma_UO]
    type = LayeredAverage
    variable = Ma
    num_layers = 10
    direction = x
  []
[]

[AuxKernels]
  [Ma_aux]
    type = NSMachAux
    variable = Ma
    fluid_properties = fp
    use_material_properties = true
  []

  [p_aux]
    type = ADMaterialRealAux
    variable = p
    property = pressure
  []

  [Ma_layered_aux]
    type = SpatialUserObjectAux
    variable = Ma_layered
    user_object = layered_Ma_UO
  []
[]

[Materials]
  [var_mat]
    type = ConservedVarValuesMaterial
    rho = rho
    rhou = rho_u
    rhov = rho_v
    rho_et = rho_E
  []
  [sound_speed]
    type = SoundspeedMat
  []
[]

[Postprocessors]
  [outflow_Ma]
    type = SideAverageValue
    variable = Ma
    boundary = right
  []

  [outflow_pressure]
    type = SideAverageValue
    variable = p
    boundary = right
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
  []
[]

[Executioner]
  type = Transient
  end_time = 10
  solve_type = NEWTON
  nl_abs_tol = 1e-7
  [TimeIntegrator]
    type = ImplicitEuler
  []

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 5e-3
    optimal_iterations = 6
    growth_factor = 1.5
  []
[]

[VectorPostprocessors]
  [Ma_layered]
    type = LineValueSampler
    variable = Ma_layered
    start_point = '0 0 0'
    end_point = '3 0 0'
    num_points = 100
    sort_by = x
    warn_discontinuous_face_values = false
  []
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'final'
  []
[]
