[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
    elem_type = HEX8
  []
  [extra_nodeset]
    type = ExtraNodesetGenerator
    input = mesh
    new_boundary = 'master'
    coord = '1.0 1.0 1.0'
  []
[]

[AuxVariables]
  [temperature]
    initial_condition = 1500
  []
  [voltage]
    initial_condition = 210
  []
[]

[AuxKernels]
  [temperature]
    type = FunctionAux
    function = temperature_function
    variable = temperature
  []
  [voltage]
    type = FunctionAux
    function = voltage_function
    variable = voltage
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Functions]
  [function_pull]
    type = PiecewiseLinear
    x = '0 100'
    y = '0 0.1'
  []
  [pressure_function]
    type = PiecewiseLinear
    x = '0 100'
    y = '0 2.0e4'
  []
  [voltage_function]
    type = PiecewiseLinear
    x = '0 100'
    y = '210 450'
  []
  [temperature_function]
    type = PiecewiseLinear
    x = '0 100'
    y = '1500 800'
  []
[]

[Constraints]
  [one]
    type = LinearNodalConstraint
    variable = disp_x
    primary = '6'
    secondary_node_ids = '1 2 5'
    penalty = 1.0e8
    formulation = kinematic
    weights = '1'
  []
  [two]
    type = LinearNodalConstraint
    variable = disp_z
    primary = '6'
    secondary_node_ids = '4 5 7'
    penalty = 1.0e8
    formulation = kinematic
    weights = '1'
  []
[]

[BCs]
  [symmy]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [symmx]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [symmz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []

  [Pressure]
    [press]
      boundary = 'top'
      displacements = 'disp_x disp_y disp_z'
      function = pressure_function
    []
  []
[]

[UserObjects]
  [uel]
    type = AbaqusUserElement
    variables = 'disp_x disp_y disp_z'
    plugin = '../../../../tensor_mechanics/examples/uel_build_tests/uel'
    use_displaced_mesh = false

    external_fields = 'temperature voltage'
    jtype = 10

    num_state_vars = 96 #
    constant_properties = '2 1 2 210000 0.3'
    extra_vector_tags = 'kernel_residual'
  []
[]

[Problem]
  kernel_coverage_check = false
  extra_tag_vectors = 'kernel_residual'
[]

[AuxVariables]
  [res_x]
  []
  [res_y]
  []
[]

[AuxKernels]
  [res_x]
    type = TagVectorAux
    variable = res_x
    v = disp_x
    vector_tag = kernel_residual
  []
  [res_y]
    type = TagVectorAux
    variable = res_y
    v = disp_y
    vector_tag = kernel_residual
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  petsc_options = '-snes_converged_reason'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = ' lu       superlu_dist'
  line_search = none

  l_max_its = 100
  l_tol = 1e-8
  nl_max_its = 1
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8

  error_on_dtmin = false
  dtmin = 10
  dt = 10
  end_time = 10
[]

[Outputs]
  exodus = true
[]
