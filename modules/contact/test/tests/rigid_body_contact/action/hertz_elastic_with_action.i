# Rigid-sphere-on-elastic-half-space Hertz test, 2D axisymmetric, small strain.
# ACTION version: uses `[Physics/SolidMechanics/QuasiStatic]` for the disp
# kernels + strain material and `[RigidContact]` for the contact stack.
# The companion `hertz_elastic_without_action.i` is the same physics
# with the contact plumbing spelled out object-by-object; both CSVDiff
# against the same gold.

[GlobalParams]
  displacements = 'disp_x disp_y'
  large_kinematics = false
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = ../../hertz_spherical/hertz_contact_rz.e
  []
  [drop_rigid_indenter]
    type = BlockDeletionGenerator
    input = file
    block = 1000
  []
  coord_type = RZ
  allow_renumbering = false
[]

[Variables]
  # SolidMechanics' `add_variables = true` would define disp_x/disp_y
  # on the block(s) it operates on; RigidContact's NCP kernel and
  # mechanical-contact BCs need the disps to ALSO be declared on the
  # lower-d block.  Since SolidMechanics doesn't split "variable block"
  # from "kernel/material block", we declare disps manually here
  # spanning both blocks, then run SolidMechanics with
  # add_variables = false and block = 1 so it only adds kernels +
  # strain material where the elasticity tensor / stress material
  # live.
  [disp_x]
    block = '1 contact_lower'
  []
  [disp_y]
    block = '1 contact_lower'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = SMALL
    add_variables = false
    new_system = true
    formulation = TOTAL
    block = 1
  []
[]

[UserObjects]
  [sphere]
    type = SphereContactor
    center = '0 -4 0'
    radius = 2.0
  []
[]

[RigidContact]
  [top]
    contactor = sphere
    boundary  = 100
    displacements = 'disp_x disp_y'
    lm_variable_name = normal_lm
    lower_d_block_name = contact_lower
  []
[]

[Materials]
  [tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1.40625e7
    poissons_ratio = 0.25
    block = 1
  []
  [stress]
    type = ComputeLagrangianLinearElasticStress
    block = 1
  []
[]

[Functions]
  [top_disp_y]
    type = PiecewiseLinear
    x = '0  1'
    y = '0 -0.01'
  []
[]

[BCs]
  [symm_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  []
  [top_deform]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 2
    function = top_disp_y
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  automatic_scaling = true

  petsc_options_iname = '-snes_type -pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'vinewtonssls lu    NONZERO               1e-12'
  line_search = semismooth

  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 40
  l_max_its = 200

  start_time = 0.0
  end_time   = 1.0
  dt         = 0.1
[]

[Postprocessors]
  [max_lm]
    type = NodalExtremeValue
    variable = normal_lm
    block = contact_lower
    value_type = max
  []
  [num_nl]
    type = NumNonlinearIterations
  []
  [cumulative_nl]
    type = CumulativeValuePostprocessor
    postprocessor = num_nl
  []
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = 'TIMESTEP_END'
  []
[]
