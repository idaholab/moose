# This example demonstrates how the mixing length model can be tuned to match an
# established correlation for pressure drop in a smooth circular pipe.

# The primary input parameters for this example are the system Reynolds number
# and the von Karman constant for the mixing length model. These two parameters
# can be changed here:
Re = 1e5
von_karman_const = 0.22

# Note that for this model (using the wall-distance mixing length for the entire
# pipe) different von Karman constants are optimal for different Reynolds
# numbers.

# This model has been non-dimensionalized. The diameter (D), density (rho), and
# bulk velocity (bulk_u) are all considered unity.
D = 1
total_len = '${fparse 40 * D}'
rho = 1
bulk_u = 1

# With those parameters set, the viscosity is then computed in order to reach
# the desired Reynolds number.
mu = '${fparse rho * bulk_u * D / Re}'

# Here the DeltaP will be evaluated by using a postprocessor to find the pressure
# at a point that is 10 diameters away from the outlet. (The outlet pressure is
# set to zero.)
L = '${fparse 10 * D}'

# We will use the McAdams correlation to find the Darcy friction factor. Note
# that this correlation is valid for fully developed flow in smooth circular
# tubes at 3e4 < Re < 1e6.
f = '${fparse 0.316 * Re^(-0.25)}'

# The DeltaP can then be computed using this friction factor as,
ref_delta_P = '${fparse f * L / D * rho * bulk_u^2 / 2}'

# Numerical parameters
advected_interp_method = 'upwind'
velocity_interp_method = 'rc'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${total_len}
    ymin = 0
    ymax = '${fparse 0.5 * D}'
    nx = 200
    ny = 40
    bias_y = '${fparse 1 / 1.2}'
  []
  [rename1]
    type = RenameBoundaryGenerator
    input = gen
    old_boundary = 'left'
    new_boundary = 'inlet'
  []
  [rename2]
    type = RenameBoundaryGenerator
    input = rename1
    old_boundary = 'right'
    new_boundary = 'outlet'
  []
  [rename3]
    type = RenameBoundaryGenerator
    input = rename2
    old_boundary = 'bottom'
    new_boundary = 'symmetry'
  []
  [rename4]
    type = RenameBoundaryGenerator
    input = rename3
    old_boundary = 'top'
    new_boundary = 'wall'
  []

  coord_type = 'RZ'
  rz_coord_axis = 'X'
[]

[Physics]
  [NavierStokes]
    [Flow]
      [all]
        compressibility = incompressible

        dynamic_viscosity = ${mu}
        density = ${rho}

        initial_pressure = 0
        initial_velocity = '1e-6 1e-6 0'

        inlet_boundaries = 'inlet'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_functors = '${bulk_u} 0.0'

        outlet_boundaries = 'outlet'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = 0.0
        wall_boundaries = 'wall       symmetry'
        momentum_wall_types = 'noslip symmetry'

        momentum_advection_interpolation = ${advected_interp_method}
        velocity_interpolation = ${velocity_interp_method}
      []
    []
    [Turbulence]
      [all]
        turbulence_handling = 'mixing-length'
        mixing_length_walls = 'wall'
        von_karman_const = ${von_karman_const}
      []
    []
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
[]

[Postprocessors]
  [delta_P]
    type = PointValue
    variable = 'pressure'
    point = '${fparse total_len - L} 0 0'
  []
  [reference_delta_P]
    type = Receiver
    default = ${ref_delta_P}
  []
[]

[Outputs]
  exodus = true
[]
