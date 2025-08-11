!include header.i
!include mesh.i

[Physics]
  [NavierStokes]
    [Flow]
      [all]
        compressibility = incompressible

        dynamic_viscosity = ${mu}
        density = ${rho}

        initial_pressure = 0
        initial_velocity = '0 0 0'

        inlet_boundaries = 'left_boundary'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_functors = 'inlet_function 0.0'
        outlet_boundaries = 'right_boundary'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = 0.0
        wall_boundaries = 'top_boundary bottom_boundary circle'
        momentum_wall_types = 'noslip noslip noslip'

        momentum_advection_interpolation = ${advected_interp_method}
      []
    []
  []
[]

!include executioner.i
!include postprocessors.i
