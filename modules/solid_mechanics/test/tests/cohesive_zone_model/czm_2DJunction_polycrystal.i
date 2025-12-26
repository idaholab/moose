[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = poly.msh
  []

  [breakmesh]
    type = BreakMeshByBlockGenerator
    input = fmg
    add_interface_on_two_sides = true
    split_interface = true
  []
  [ss]
    type = SideSetsFromNormalsGenerator
    input = breakmesh
    normals = '1  0  0
              -1  0  0
              0 -1 0'
    new_boundary = 'right left bottom'
  []
  parallel_type = distributed
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
  use_displaced_mesh = false
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

# [Physics]

#   [SolidMechanics]

#     [CohesiveZone]
#       [czm_ik]
#         boundary = 'Block1_Block2 Block1_Block6 Block1_Block7 Block2_Block5 Block2_Block6 Block2_Block7 Block2_Block10 Block3_Block4 Block3_Block5 Block4_Block5 Block4_Block8 Block4_Block10 Block5_Block7 Block5_Block10 Block6_Block8 Block6_Block9 Block6_Block10 Block8_Block9 Block8_Block10'
#       []
#     []
#   []
# []

[Materials]
  [elastic_stress]
    type = ADComputeLinearElasticStress
  []
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 1000
  []

  [jump]
    type = ADCZMComputeDisplacementJumpSmallStrain
    # displacements added through GlobalParams
    boundary = 'Block1_Block2 Block1_Block6 Block1_Block7 Block2_Block5 Block2_Block6 Block2_Block7 Block2_Block10 Block3_Block4 Block3_Block5 Block4_Block5 Block4_Block8 Block4_Block10 Block5_Block7 Block5_Block10 Block6_Block8 Block6_Block9 Block6_Block10 Block8_Block9 Block8_Block10'
  []
  [interface_traction]
    type = ADPureElasticTractionSeparation
    normal_stiffness = 1e4
    tangent_stiffness = 7e3
    boundary = 'Block1_Block2 Block1_Block6 Block1_Block7 Block2_Block5 Block2_Block6 Block2_Block7 Block2_Block10 Block3_Block4 Block3_Block5 Block4_Block5 Block4_Block8 Block4_Block10 Block5_Block7 Block5_Block10 Block6_Block8 Block6_Block9 Block6_Block10 Block8_Block9 Block8_Block10'
  []
  [global_traction]
    type = ADCZMComputeGlobalTractionSmallStrain
    boundary = 'Block1_Block2 Block1_Block6 Block1_Block7 Block2_Block5 Block2_Block6 Block2_Block7 Block2_Block10 Block3_Block4 Block3_Block5 Block4_Block5 Block4_Block8 Block4_Block10 Block5_Block7 Block5_Block10 Block6_Block8 Block6_Block9 Block6_Block10 Block8_Block9 Block8_Block10'
  []
[]

[InterfaceKernels]
  [sczm_x]
    type = ADCZMInterfaceKernelSmallStrain
    variable = disp_x
    neighbor_var = disp_x
    component = 0
    boundary = 'Block1_Block2 Block1_Block6 Block1_Block7 Block2_Block5 Block2_Block6 Block2_Block7 Block2_Block10 Block3_Block4 Block3_Block5 Block4_Block5 Block4_Block8 Block4_Block10 Block5_Block7 Block5_Block10 Block6_Block8 Block6_Block9 Block6_Block10 Block8_Block9 Block8_Block10'
  []
  [sczm_y]
    type = ADCZMInterfaceKernelSmallStrain
    variable = disp_y
    neighbor_var = disp_y
    component = 1
    boundary = 'Block1_Block2 Block1_Block6 Block1_Block7 Block2_Block5 Block2_Block6 Block2_Block7 Block2_Block10 Block3_Block4 Block3_Block5 Block4_Block5 Block4_Block8 Block4_Block10 Block5_Block7 Block5_Block10 Block6_Block8 Block6_Block9 Block6_Block10 Block8_Block9 Block8_Block10'
  []
[]

[Outputs]
  exodus = true
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

[Functions]
  [displacement_with_time]
    type = ParsedFunction
    expression = '1e-3*t'
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

