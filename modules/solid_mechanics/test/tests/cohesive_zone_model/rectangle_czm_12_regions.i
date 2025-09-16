
# nx = 1024
poi = 0.3
E1 = 1e3

[GlobalParams]
  displacements = 'disp_x disp_y'
  use_displaced_mesh = false
[]

[Problem]
[]

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.2 0.3 0.5'
    dy = '0.3 0.3 0.3 0.1'
    ix = '6 9 15'
    iy = '9 9 9 3'

    subdomain_id = '1 2 3 4 5 6 7 8 9 10 11 12'
  []

  [break]
    type = BreakMeshByBlockGenerator
    input = gen
    split_interface = true
    add_interface_on_two_sides = true
  []

  parallel_type = distributed
[]

[Outputs]
  exodus = true
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
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    poissons_ratio = ${poi}
    youngs_modulus = ${E1}
  []
  [jump]
    type = ADCZMComputeDisplacementJumpSmallStrain
    # displacements added through GlobalParams
    boundary = 'Block1_Block2 Block1_Block4 Block2_Block3 Block3_Block6 Block4_Block5 Block4_Block7 Block5_Block6 Block5_Block8 Block6_Block9 Block7_Block8 Block7_Block10 Block8_Block9 Block8_Block11 Block9_Block12 Block10_Block11 Block11_Block12'
  []
  [interface_traction]
    type = ADPureElasticTractionSeparation
    normal_stiffness = 1e4
    tangent_stiffness = 7e3
    boundary = 'Block1_Block2 Block1_Block4 Block2_Block3 Block3_Block6 Block4_Block5 Block4_Block7 Block5_Block6 Block5_Block8 Block6_Block9 Block7_Block8 Block7_Block10 Block8_Block9 Block8_Block11 Block9_Block12 Block10_Block11 Block11_Block12'
  []
  [global_traction]
    type = ADCZMComputeGlobalTractionSmallStrain
    boundary = 'Block1_Block2 Block1_Block4 Block2_Block3 Block3_Block6 Block4_Block5 Block4_Block7 Block5_Block6 Block5_Block8 Block6_Block9 Block7_Block8 Block7_Block10 Block8_Block9 Block8_Block11 Block9_Block12 Block10_Block11 Block11_Block12'
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
    expression = '5e-3*t'
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
    boundary = 'Block1_Block2 Block1_Block4 Block2_Block3 Block3_Block6 Block4_Block5 Block4_Block7 Block5_Block6 Block5_Block8 Block6_Block9 Block7_Block8 Block7_Block10 Block8_Block9 Block8_Block11 Block9_Block12 Block10_Block11 Block11_Block12'
  []
  [sczm_y]
    type = ADCZMInterfaceKernelSmallStrain
    variable = disp_y
    neighbor_var = disp_y
    component = 1
    boundary = 'Block1_Block2 Block1_Block4 Block2_Block3 Block3_Block6 Block4_Block5 Block4_Block7 Block5_Block6 Block5_Block8 Block6_Block9 Block7_Block8 Block7_Block10 Block8_Block9 Block8_Block11 Block9_Block12 Block10_Block11 Block11_Block12'
  []
[]

[AuxVariables]
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
