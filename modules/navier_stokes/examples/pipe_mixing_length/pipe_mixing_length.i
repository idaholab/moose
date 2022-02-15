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
total_len = ${fparse 40 * D}
rho = 1
bulk_u = 1

# With those parameters set, the viscosity is then computed in order to reach
# the desired Reynolds number.
mu = ${fparse rho * bulk_u * D / Re}

# Here the DeltaP will be evaluted by using a postprocessor to find the pressure
# at a point that is 10 diameters away from the outlet. (The outlet pressure is
# set to zero.)
L = ${fparse 10 * D}

# We will use the McAdams correlation to find the Darcy friction factor. Note
# that this correlation is valid for fully developed flow in smooth circular
# tubes at 3e4 < Re < 1e6.
f = ${fparse 0.316 * Re^(-0.25)}

# The DeltaP can then be computed using this friction factor as,
ref_delta_P = ${fparse f * L / D * rho * bulk_u^2 / 2}

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${total_len}
    ymin = 0
    ymax = ${fparse 0.5 * D}
    nx = 200
    ny = 40
    bias_y = ${fparse 1 / 1.2}
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
  coord_type = 'RZ'
  rz_coord_axis = 'X'
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
  # The upwind and Rhie-Chow interpolation schemes are used here.
  advected_interp_method='upwind'
  velocity_interp_method='rc'
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
  [mixing_len]
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
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_viscosity_rans]
    type = INSFVMixingLengthReynoldsStress
    variable = u
    rho = ${rho}
    mixing_length = mixing_len
    momentum_component = 'x'
    u = u
    v = v
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
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_viscosity_rans]
    type = INSFVMixingLengthReynoldsStress
    variable = v
    rho = ${rho}
    mixing_length = mixing_len
    momentum_component = 'y'
    u = u
    v = v
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    pressure = pressure
  []
[]

[AuxKernels]
  [mixing_len]
    type = WallDistanceMixingLengthAux
    walls = 'wall'
    variable = mixing_len
    execute_on = 'initial'
    von_karman_const = ${von_karman_const}
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
