[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    ymax = 2
    nx = 5
    ny = 10
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    input = 'msh'
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 1
    block_name = 'block1'
  []
  [block2]
    type = SubdomainBoundingBoxGenerator
    input = 'block1'
    bottom_left = '0 1 0'
    top_right = '1 2 0'
    block_id = 2
    block_name = 'block2'
  []
  [split]
    type = BreakMeshByBlockGenerator
    input = block2
  []
  [top_node]
    type = ExtraNodesetGenerator
    coord = '0 2 0'
    input = split
    new_boundary = top_node
  []
  [bottom_node]
    type = ExtraNodesetGenerator
    coord = '0 0 0'
    input = top_node
    new_boundary = bottom_node
  []
  use_checkpoint_mesh = false
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Modules]
  [TensorMechanics]
    [Master]
      generate_output = 'stress_yy'
      [all]
        strain = FINITE
        add_variables = true
        use_automatic_differentiation = true
        decomposition_method = TaylorExpansion
        save_in = 'resid_x resid_y'
      []
    []
  []
[]

[ICs]
  [scale_factor]
    type = FunctionIC
    variable = scale_factor
    function = 'if(x<0.5,1,100)'
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    preset = true
    value = 0.0
    boundary = bottom_node
    variable = disp_x
  []

  [fix_top]
    type = DirichletBC
    preset = true
    boundary = top
    variable = disp_x
    value = 0
  []

  [top]
    type = FunctionDirichletBC
    boundary = top
    variable = disp_y
    function = 'if(t<=0.3,t,if(t<=0.6,0.3-(t-0.3),0.6-t))'
    preset = true
  []

  [bottom]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0
    preset = true
  []
[]

[AuxVariables]
  [resid_x]
  []
  [resid_y]
  []
  [mode_mixity_ratio]
    order = CONSTANT
    family = MONOMIAL
  []
  [damage]
    order = CONSTANT
    family = MONOMIAL
  []
  [effective_displacement_at_full_degradation]
    order = CONSTANT
    family = MONOMIAL
  []
  [maximum_mixed_mode_relative_displacement]
    order = CONSTANT
    family = MONOMIAL
  []
  [effective_displacement_at_damage_initiation]
    order = CONSTANT
    family = MONOMIAL
  []
  [scale_factor]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [mode_mixity_ratio]
    type = MaterialRealAux
    variable = mode_mixity_ratio
    property = mode_mixity_ratio
    execute_on = timestep_end
    boundary = interface
  []
  [damage]
    type = MaterialRealAux
    variable = damage
    property = damage
    execute_on = timestep_end
    boundary = interface
  []
  [effective_displacement_at_damage_initiation]
    type = MaterialRealAux
    variable = effective_displacement_at_damage_initiation
    property = effective_displacement_at_damage_initiation
    execute_on = timestep_end
    boundary = interface
  []
  [effective_displacement_at_full_degradation]
    type = MaterialRealAux
    variable = effective_displacement_at_full_degradation
    property = effective_displacement_at_full_degradation
    execute_on = timestep_end
    boundary = interface
  []
  [maximum_mixed_mode_relative_displacement]
    type = MaterialRealAux
    variable = maximum_mixed_mode_relative_displacement
    property = maximum_mixed_mode_relative_displacement
    execute_on = timestep_end
    boundary = interface
  []
[]

[Modules/TensorMechanics/CohesiveZoneMaster]
  [czm_ik]
    boundary = 'interface'
  []
[]

[Materials]
  [stress]
    type = ADComputeFiniteStrainElasticStress
  []
  [elasticity_tensor]
    type = ADComputeElasticityTensor
    fill_method = symmetric9
    C_ijkl = '1.684e5 0.176e5 0.176e5 1.684e5 0.176e5 1.684e5 0.754e5 0.754e5 0.754e5'
  []
  [czm]
    type = BiLinearMixedModeTraction
    boundary = 'interface'
    penalty_stiffness = 1e6
    GI_C = 1e3
    GII_C = 1e2
    normal_strength = 1e4
    shear_strength = 1e3
    displacements = 'disp_x disp_y'
    eta = 2.2
    viscosity = 1e-3
    normal_strength_scale_factor = scale_factor
  []
[]

[Postprocessors]
  [resid_x]
    type = NodalSum
    variable = resid_x
    boundary = top
  []
  [resid_y]
    type = NodalSum
    variable = resid_y
    boundary = top
  []
  [disp_y]
    type = SideAverageValue
    variable = disp_y
    boundary = top
  []
  [disp_x]
    type = SideAverageValue
    variable = disp_x
    boundary = top
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

  solve_type = 'NEWTON'
  line_search = none

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  automatic_scaling = true

  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 30
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  start_time = 0.0
  dt = 0.01
  end_time = 0.05
  dtmin = 0.01
[]

[Outputs]
  csv = true
  exodus = true
[]
