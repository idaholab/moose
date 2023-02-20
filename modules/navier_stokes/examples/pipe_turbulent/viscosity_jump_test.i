# This example demonstrates how the mixing length model can be tuned to match an
# established correlation for pressure drop in a smooth circular pipe.

# The primary input parameters for this example are the system Reynolds number
# and the von Karman constant for the mixing length model. These two parameters
# can be changed here:
Re = 1e5

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

# Here the DeltaP will be evaluted by using a postprocessor to find the pressure
# at a point that is 10 diameters away from the outlet. (The outlet pressure is
# set to zero.)
L = '${fparse 10 * D}'

# Discretization
nx = 40
ny = 20

# Crafted wall function
f = '${fparse 0.316 * Re^(-0.25)}'
ref_delta_P = '${fparse f * L / D * rho * bulk_u^2 / 2}'
tau_wall = '${fparse ref_delta_P / (pi * D * L)}'
u_tau = '${fparse sqrt(tau_wall / rho)}'
y_dist_wall = '${fparse D/(4*ny)}'
mu_wall = '${fparse rho * pow(u_tau,2) * y_dist_wall / bulk_u}'

# Crafted bulk viscosity
turbulent_intensity = 0.1
c_mu = 0.09
mixing_length = '${fparse D * 0.07}'
k_bulk = '${fparse 3/2 * pow(bulk_u*turbulent_intensity, 2)}'
eps_bulk = '${fparse pow(c_mu, 0.75) * pow(k_bulk, 1.5) / mixing_length}'
mu_bulk = '${fparse c_mu * pow(k_bulk, 2) / eps_bulk}'

[Mesh]
  coord_type = 'RZ'
  rz_coord_axis = 'X'
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${total_len}
    ymin = 0
    ymax = '${fparse 0.5 * D}'
    nx = ${nx}
    ny = ${ny}
    bias_y = '${fparse 1 / 1.0}'
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
[]

[Outputs]
  exodus = true
[]

[Problem]
  kernel_coverage_check = false
  fv_bcs_integrity_check = true
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = 'upwind'
  velocity_interp_method = 'average'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = u
    v = v
    pressure = pressure
  []
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
  []
  [v]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[AuxVariables]
  [viscous_field]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    rho = ${rho}
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = u
    mu = 'mu'
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    pressure = pressure
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = v
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = v
    mu = 'mu'
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    pressure = pressure
  []
[]

[AuxKernels]
  [populate_viscous_field]
    type = ADFunctorElementalAux
    variable = viscous_field
    functor = 'mu'
  []
[]

[Functions]
  # Not working
  [viscous_jump]
    type = ADParsedFunction
    expression = 'if((y > (0.5 * D)*(ny -1)/ny), mu_wall, mu_bulk)'
    symbol_names = 'D ny mu_wall mu_bulk'
    symbol_values = '${D} ${ny} ${mu_wall} ${mu_bulk}'
  []
  # Working
  # [viscous_jump]
  #   type = ADParsedFunction
  #   expression = 'if((y > (0.5 * D)*(ny -1)/ny), mu_wall*10.0, mu_bulk)'
  #   symbol_names = 'D ny mu_wall mu_bulk'
  #   symbol_values = '${D} ${ny} ${mu_wall} ${mu_bulk}'
  # []
[]

[Materials]
  [viscosity]
    type = ADGenericFunctorMaterial
    prop_names = 'mu'
    prop_values = 'viscous_jump'
  []
[]

[FVBCs]
  [inlet_u]
    type = INSFVInletVelocityBC
    boundary = 'inlet'
    variable = u
    function = ${bulk_u}
  []
  [inlet_v]
    type = INSFVInletVelocityBC
    boundary = 'inlet'
    variable = v
    function = '0'
  []
  [walls_u]
    type = INSFVNoSlipWallBC
    boundary = 'wall'
    variable = u
    function = 0
  []
  [walls_v]
    type = INSFVNoSlipWallBC
    boundary = 'wall'
    variable = v
    function = 0
  []
  [sym_u]
    type = INSFVSymmetryVelocityBC
    boundary = 'symmetry'
    variable = u
    u = u
    v = v
    mu = ${mu}
    momentum_component = x
  []
  [sym_v]
    type = INSFVSymmetryVelocityBC
    boundary = 'symmetry'
    variable = v
    u = u
    v = v
    mu = ${mu}
    momentum_component = y
  []
  [sym_p]
    type = INSFVSymmetryPressureBC
    boundary = 'symmetry'
    variable = pressure
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'outlet'
    variable = pressure
    function = '0'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  # petsc_options = '-snes_monitor -ksp_monitor -snes_converged_reason -ksp_converged_reason'
  # petsc_options_iname = '-pc_type -pc_svd_monitor'
  # petsc_options_value = 'svd true'
  line_search = none # working better with default or l2 line search
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
[]

[Postprocessors]
  [delta_P]
    type = PointValue
    variable = 'pressure'
    point = '${fparse total_len - L} 0 0'
  []
[]
