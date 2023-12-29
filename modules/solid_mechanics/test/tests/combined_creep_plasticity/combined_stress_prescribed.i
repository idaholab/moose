#
# 1x1x1 unit cube with time-varying pressure on top face
#
# The problem is a one-dimensional creep analysis.  The top face has a
#    pressure load that is a function of time.  The creep strain can be
#    calculated analytically.  There is no practical active linear
#    isotropic plasticity because the yield stress for the plasticity
#    model is set to 1e30 MPa, which will not be reached in this
#    regression test.
#
# The analytic solution to this problem is:
#
#    d ec
#    ---- = a*S^b  with S = c*t^d
#     dt
#
#    d ec = a*c^b*t^(b*d) dt
#
#         a*c^b
#    ec = ----- t^(b*d+1)
#         b*d+1
#
#    where S  = stress
#          ec = creep strain
#          t  = time
#          a  = constant
#          b  = constant
#          c  = constant
#          d  = constant
#
# With a = 3e-24,
#      b = 4,
#      c = 1,
#      d = 1/2, and
#      t = 32400
#   we have
#
#   S = t^(1/2) = 180
#
#   ec = 1e-24*t^3 = 3.4012224e-11
#
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    incremental = true
    add_variables = true
    generate_output = 'stress_yy creep_strain_yy'
  [../]
[]

[Functions]
  [./pressure]
    type = ParsedFunction
    expression = 'sqrt(t)'
  [../]

  [./dts]
    type = PiecewiseLinear
    y = '1e-2 1e-1 1e0 1e1 1e2'
    x = '0    7e-1 7e0 7e1 1e2'
  [../]
[]

[BCs]
  [./top_pressure]
    type = Pressure
    variable = disp_y
    boundary = top
    function = pressure
  [../]
  [./u_bottom_fix]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./u_yz_fix]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./u_xy_fix]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.8e7
    poissons_ratio = 0.3
  [../]
  [./creep_plas]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'creep plas'
    tangent_operator = elastic
  [../]
  [./creep]
    type = PowerLawCreepStressUpdate
    coefficient = 3.0e-24
    n_exponent = 4
    m_exponent = 0
    activation_energy = 0
  [../]
  [./plas]
    type = IsotropicPlasticityStressUpdate
    hardening_constant = 1
    yield_stress = 1e30
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = ' lu       superlu_dist'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-7
  l_tol = 1e-6
  start_time = 0.0
  end_time = 32400
  dt = 1e-2
  [./TimeStepper]
    type = FunctionDT
    function = dts
  [../]
[]

[Postprocessors]
  [./timestep]
    type = TimestepSize
  [../]
[]

[Outputs]
  exodus = true
[]
