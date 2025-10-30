# nx = 1024
nx = 16
nx_half = '${fparse nx/2}'
E1 = 1e3
E2 = 1e3
poi = 0.3
x0 = 0.5
x0_double = '${fparse 2*x0}'

[GlobalParams]
  displacements = 'disp_x disp_y'
  use_displaced_mesh = false
[]

[Problem]
  extra_tag_vectors = 'ref'
[]

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${x0} ${x0}'
    dy = '${x0_double}'
    ix = '${nx_half} ${nx_half}'
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

  parallel_type = distributed
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
        extra_vector_tags = 'ref'
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
    normal_stiffness = 1e4
    tangent_stiffness = 7e3
    boundary = 'Block1_Block2'
  []
  [global_traction]
    type = ADCZMComputeGlobalTractionSmallStrain
    boundary = 'Block1_Block2'
  []
[]

[Executioner]
  type = Transient
  # solve_type = FD
  # automatic_scaling = false
  # residual_and_jacobian_together = false
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu superlu_dist'
  automatic_scaling = true
  # residual_and_jacobian_together = true
  line_search = 'none'

  nl_max_its = 500
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-50
  dt = 1
  end_time = 200
  abort_on_solve_fail = true
[]

[Functions]
  [displacement_with_time]
    type = ParsedFunction
    # expression = '1e-2*sin(pi*t/160)*t'
    expression = '1e-2*t'
  []
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

[AuxKernels]
  [react_x]
    type = TagVectorAux
    vector_tag = 'ref'
    v = 'disp_x'
    variable = 'react_x'
    scaled = false
  []
  [react_y]
    type = TagVectorAux
    vector_tag = 'ref'
    v = 'disp_y'
    variable = 'react_y'
    scaled = false
  []
[]

[BCs]
  [anchor_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0.0
  []
  [anchor_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0.0
  []
  [displacement_x_right]
    #Anchors the left side against deformation in the x-direction
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'right'
    function = displacement_with_time
    preset = false
  []
[]

[Postprocessors]
  [react_x]
    type = NodalSum
    variable = 'react_x'
    boundary = 'right'
  []
  [react_y]
    type = NodalSum
    variable = 'react_y'
    boundary = 'right'
  []
  [length]
    type = AreaPostprocessor
    boundary = 'right'
  []
[]

[Outputs]
  exodus = true
  [csv]
    type = CSV
    precision = 15
  []
[]
