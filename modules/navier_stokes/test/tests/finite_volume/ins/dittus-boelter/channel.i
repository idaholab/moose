# Fluid properties
mu = 1
rho = 1
cp = 1
k = 1
D_h = 2
num_axial_elements = 50

# Operating conditions
u_inlet = 1
T_inlet = 300
T_wall = 400
p_outlet = 0

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 5
    ymin = 0
    ymax = '${fparse D_h/2}'
    nx = ${num_axial_elements}
    ny = 20
  []
[]

[Physics]
  [NavierStokes]
    [Flow]
      [flow]
        compressibility = 'incompressible'

        density = 'rho'
        dynamic_viscosity = 'mu'

        initial_velocity = '${u_inlet} 0 0'
        initial_pressure = 0.0

        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_functors = '${u_inlet} 0'
        wall_boundaries = 'bottom top'
        momentum_wall_types = 'symmetry noslip'

        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure-zero-gradient'
        pressure_functors = '${p_outlet}'

        mass_advection_interpolation = 'average'
        momentum_advection_interpolation = 'average'
      []
    []
    [FluidHeatTransfer]
      [heat]
        thermal_conductivity = 'k'
        specific_heat = 'cp'

        fluid_temperature_variable = 'T_fluid'
        initial_temperature = '${T_inlet}'
        energy_inlet_types = 'heatflux'
        energy_inlet_functors = '${fparse u_inlet * rho * cp * T_inlet}'

        energy_wall_types = 'heatflux heatflux'
        energy_wall_functors = '0 q'

        energy_advection_interpolation = 'average'
      []
    []
  []
[]

[FluidProperties]
  [simple]
    type = SimpleFluidProperties
    thermal_conductivity = ${k}
    cp = ${cp}
    viscosity = ${mu}
    density0 = ${rho}
  []
[]

[UserObjects]
  [layered_speed]
    execute_on = 'linear nonlinear'
    type = LayeredAverageFunctor
    direction = 'x'
    functor = 'speed'
    num_layers = ${num_axial_elements}
  []
  [layered_T_fluid]
    execute_on = 'linear nonlinear'
    type = LayeredAverageFunctor
    direction = 'x'
    num_layers = ${num_axial_elements}
    functor = 'T_fluid'
  []
[]

[FunctorMaterials]
  [converter]
    type = FunctorADConverter
    ad_props_in = 'pressure'
    reg_props_out = 'nonad_pressure'
  []
  [functor_props]
    type = NonADGeneralFunctorFluidProps
    T_fluid = layered_T_fluid
    characteristic_length = ${D_h}
    fp = simple
    porosity = 1
    pressure = nonad_pressure
    speed = layered_speed
  []
  [dittus]
    type = DittusBoelterFunctorMaterial
    D_h = ${D_h}
    Hw = Hw
    Pr = Pr
    Re = Re
    T_fluid = layered_T_fluid
    T_wall = ${T_wall}
    k = k
  []
  [q]
    type = ParsedFunctorMaterial
    expression = 'Hw * (T_wall - T_fluid)'
    functor_symbols = 'Hw T_fluid T_wall'
    functor_names = 'Hw layered_T_fluid ${T_wall}'
    property_name = 'q'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  line_search = 'none'
[]

[Outputs]
  exodus = true
[]
