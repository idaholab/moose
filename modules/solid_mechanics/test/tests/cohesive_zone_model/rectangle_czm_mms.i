# nx = 1024
nx = 16
nx_quad = '${fparse nx/4}'
nx_quad_triple = '${fparse 3*nx_quad}'
E1 = 1e3
E2 = 1e3
poi = 0.3
x0 = 0.25
x0_triple = '${fparse 3*x0}'
x_domain = 1
a = '${fparse (pi * cos(pi * x0) * (1 - E1 / E2)) / (2 * x0)}'
b = '${fparse -pi * cos(pi * x0) * (E1 + (x0 / 2) * (1 - E1 / E2))}'

[GlobalParams]
  displacements = 'disp_x disp_y'
  use_displaced_mesh = false
[]

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${x0} ${x0_triple}'
    dy = '${x_domain}'
    ix = '${nx_quad} ${nx_quad_triple}'
    iy = '${nx}'
    subdomain_id = '1 2'
  []

  [break]
    type = BreakMeshByBlockGenerator
    input = gen
    split_interface = true
    add_interface_on_two_sides = true
    # prepare_end = false
  []

  # parallel_type = distributed
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = SMALL
        use_automatic_differentiation = true
        generate_output = 'stress_xx stress_xy stress_yy stress_zz strain_xx strain_xy strain_yy strain_zz'
      []
    []
  []
[]

[Materials]
  [elastic_stress]
    type = ADComputeLinearElasticStress
  []
  [elasticity_tensor_in]
    type = ADComputeIsotropicElasticityTensor
    poissons_ratio = ${poi}
    youngs_modulus = ${E1}
    block = 1
  []
  [elasticity_tensor_out]
    type = ADComputeIsotropicElasticityTensor
    poissons_ratio = ${poi}
    youngs_modulus = ${E2}
    block = 2
  []
  [jump]
    type = ADCZMComputeDisplacementJumpSmallStrain
    # displacements added through GlobalParams
    boundary = 'Block1_Block2'
  []
  [interface_traction]
    type = ADPureElasticTractionSeparation
    normal_stiffness = 1
    tangent_stiffness = 0
    boundary = 'Block1_Block2'
  []
  [global_traction]
    type = ADCZMComputeGlobalTractionSmallStrain
    boundary = 'Block1_Block2'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu superlu_dist'
  automatic_scaling = true
  line_search = 'none'
[]

[Kernels]
[]

[InterfaceKernels]
  [sczm_x]
    type = ADCZMInterfaceKernelSmallStrain
    variable = disp_x
    neighbor_var = disp_x
    component = 0
    boundary = 'Block1_Block2'
  []
  [sczm_y]
    type = ADCZMInterfaceKernelSmallStrain
    variable = disp_y
    neighbor_var = disp_y
    component = 1
    boundary = 'Block1_Block2'
  []
[]

[AuxVariables]
  [react_x]
  []
  [react_y]
  []
  [proc]
    [AuxKernel]
      type = ProcessorIDAux
      execute_on = initial
    []
  []
  [proc_elem]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = ProcessorIDAux
      execute_on = initial
    []
  []
[]

[Functions]
  [ux_func_left]
    type = ParsedFunction
    expression = '-sin(pi*x)'
  []

  [ux_func_right]
    type = ParsedFunction
    expression = '-sin(pi*x)+${a}*x^2+${b}'
  []

  [bx_func_left]
    type = ParsedFunction
    expression = '-pi^2*${E1}*sin(pi*x)'
  []

  [bx_func_right]
    type = ParsedFunction
    expression = '-${E2}*(2*${a} + pi^2*sin(pi*x))'
  []
[]

[Kernels]
  [source_x_left]
    type = BodyForce
    variable = disp_x
    function = bx_func_left
    block = 1
  []
  [source_x_right]
    type = BodyForce
    variable = disp_x
    function = bx_func_right
    block = 2
  []
[]

[BCs]
  [y_anchor]
    type = DirichletBC
    variable = disp_y
    boundary = 'left right'
    value = 0.0
  []

  [x_left]
    type = FunctionDirichletBC
    boundary = 'left'
    function = ux_func_left
    variable = disp_x
  []

  [x_right]
    type = FunctionDirichletBC
    boundary = 'right'
    function = ux_func_right
    variable = disp_x
  []
[]

[Postprocessors]
  [l2_error_left_x]
    type = ElementL2Error
    variable = disp_x
    function = ux_func_left
    block = '1'
  []

  [l2_error_right_x]
    type = ElementL2Error
    variable = disp_x
    function = ux_func_right
    block = '2'
  []
[]

[Outputs]
  exodus = true
  [csv]
    type = CSV
    precision = 15
  []
[]
