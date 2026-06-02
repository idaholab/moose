[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 4
  ny = 4
  elem_type = QUAD4
[]

[Variables]
  [lambda_left_x]
    family = SCALAR
    order = FIRST
  []
  [lambda_bottom_x]
    family = SCALAR
    order = FIRST
  []
  [lambda_bottom_y]
    family = SCALAR
    order = FIRST
  []
  [lambda_top_y]
    family = SCALAR
    order = FIRST
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = SMALL
    planar_formulation = PLANE_STRAIN
    add_variables = true
  []
[]

[BCs]
  [left_x_average]
    type = BoundaryIntegralValueConstraint
    variable = disp_x
    boundary = left
    lambda = lambda_left_x
  []
  [bottom_x_average]
    type = BoundaryIntegralValueConstraint
    variable = disp_x
    boundary = bottom
    lambda = lambda_bottom_x
  []
  [bottom_y_average]
    type = BoundaryIntegralValueConstraint
    variable = disp_y
    boundary = bottom
    lambda = lambda_bottom_y
  []
  [top_y_average]
    type = BoundaryIntegralValueConstraint
    variable = disp_y
    boundary = top
    lambda = lambda_top_y
    phi0 = 1e-3
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.25
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[Postprocessors]
  [left_x_average]
    type = SideAverageValue
    variable = disp_x
    boundary = left
    execute_on = timestep_end
  []
  [bottom_x_average]
    type = SideAverageValue
    variable = disp_x
    boundary = bottom
    execute_on = timestep_end
  []
  [bottom_y_average]
    type = SideAverageValue
    variable = disp_y
    boundary = bottom
    execute_on = timestep_end
  []
  [top_y_average]
    type = SideAverageValue
    variable = disp_y
    boundary = top
    execute_on = timestep_end
  []
[]

[Preconditioning]
  [full]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-12
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
[]

[Outputs]
  csv = true
  hide = 'lambda_left_x lambda_bottom_x lambda_bottom_y lambda_top_y'
[]
