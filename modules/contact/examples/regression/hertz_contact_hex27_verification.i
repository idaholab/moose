# Hertz Contact: Sphere on plane

# One sphere has an infinite radius (plane), Young's modulus, and Poisson's ratio.

# Define E:
# 1/E = (1-nu1^2)/E1 + (1-nu2^2)/E2
# For this problem:
# E = 15 MPa

# Effective radius R:
# 1/R = 1/R1 + 1/R2
# R = 2 m
#
# F is the applied compressive load.
#
# Area of contact a:
# a = sqrt(R * d), where "d" is the indentation
# F = 4/3 * E * sqrt(R) * d^{3/2}

# In the simulation, we enforce d = 0.01m
# F_{theory} = 28.2243 kN
# The reaction force from the simulation is
# F_{simulation} = 72.8952 kN * 4 = 29.158 kN

# Error is: 3.3%

[GlobalParams]
  volumetric_locking_correction = false
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh] #Comment
  file = hertz_contact_hex27_verification.e
[]

[Functions]
  [pressure]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 1.'
    scale_factor = 795.77471545947674
  []
  [disp_y]
    type = PiecewiseLinear
    x = '0.  1.    2.'
    y = '0. -0.01 -0.01' # Functions
  []
[]

[Variables]
  [disp_x]
    order = SECOND
    family = LAGRANGE
  []
  [disp_y]
    order = SECOND
    family = LAGRANGE
  []
  [disp_z]
    order = SECOND
    family = LAGRANGE
  []
[]

[AuxVariables]
  [saved_x]
    order = SECOND
    family = LAGRANGE
  []
  [saved_y]
    order = SECOND
    family = LAGRANGE
  []
  [saved_z]
    order = SECOND
    family = LAGRANGE
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = SMALL
    save_in = 'saved_x saved_y saved_z'
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx vonmises_stress '
                      'hydrostatic_stress'
  []
[]

[AuxKernels]
[]

[BCs]
  [base_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1000
    value = 0.0
  []
  [base_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1000
    value = 0.0
  []
  [base_z]
    type = DirichletBC
    variable = disp_z
    boundary = 1000
    value = 0.0
  []

  [symm_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  []
  [symm_z]
    type = DirichletBC
    variable = disp_z
    boundary = 3
    value = 0.0
  []
  [disp_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 2
    function = disp_y
  []
[]

[Contact]
  [dummy_name]
    primary = 1000
    secondary = 100
    tension_release = 0.0
    normalize_penalty = true
    tangential_tolerance = 1e-1
    penalty = 1e+10
    formulation = penalty
  []
[]

[Materials]
  [tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1.40625e7
    poissons_ratio = 0.25
  []
  [stress]
    type = ComputeLinearElasticStress
    block = '1'
  []

  [tensor_1000]
    type = ComputeIsotropicElasticityTensor
    block = '1000'
    youngs_modulus = 1e6
    poissons_ratio = 0.0
  []
  [stress_1000]
    type = ComputeLinearElasticStress
    block = '1000'
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'
  line_search = 'none'
  nl_abs_tol = 1e-7
  l_max_its = 10
  start_time = 0.0
  dt = 0.5
  end_time = 2.0
[]

[Postprocessors]
  [maxdisp]
    type = NodalVariableValue
    nodeid = 1082
    variable = disp_y
  []
  [resid_y]
    type = NodalSum
    variable = saved_y
    boundary = 2
  []
[]

[Outputs]
  [out]
    type = Exodus
    elemental_as_nodal = true
  []
  [chkfile]
    type = CSV
    file_base = hertz_contact_hex27_verification_chkfile
    show = 'maxdisp resid_y'
    execute_on = 'FINAL'
  []
  perf_graph = true
[]
