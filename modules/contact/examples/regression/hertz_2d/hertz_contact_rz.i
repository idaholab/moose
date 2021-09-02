[GlobalParams]
  volumetric_locking_correction = true
  displacements = 'disp_x disp_y'
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = hertz_contact_rz_verification_.e
  displacements = 'disp_x disp_y'
[]

[Functions]
  [disp_y]
    type = PiecewiseLinear
    x = '0.  1.    2.'
    y = '0. -0.01 -0.01'
  []
[]

[Variables]
  [disp_x]
    order = FIRST
    family = LAGRANGE
  []

  [disp_y]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [saved_x]
    order = FIRST
    family = LAGRANGE
  []
  [saved_y]
    order = FIRST
    family = LAGRANGE
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = SMALL
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_xz stress_yz vonmises_stress '
                      'hydrostatic_stress'
    save_in = 'saved_x saved_y'
  []
[]

[AuxKernels]

[]

[BCs]
  [base_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1000
    value = 0.0
  []
  [symm_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
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
    model = frictionless
    formulation = kinematic
    normalize_penalty = true
    # friction_coefficient = 0.4
    penalty = 8e7
    tangential_tolerance = 0.005
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
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  line_search = 'none'
  nl_abs_tol = 1e-8
  l_max_its = 200
  start_time = 0.0
  dt = 0.1
  end_time = 2.0 # Executioner
[]

[Postprocessors]
  [maxdisp]
    type = NodalVariableValue
    nodeid = 3200
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
    file_base = hertz_contact_rz_chkfile
    show = 'maxdisp resid_y'
    execute_on = 'FINAL'
  []
[]
