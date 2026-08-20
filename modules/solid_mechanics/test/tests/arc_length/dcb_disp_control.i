length = 100
arm = 3
precrack = 30
nx = 100

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = ${length}
    ymax = ${fparse 2 * arm}
    nx = ${nx}
    ny = ${fparse 2 * arm}
    elem_type = QUAD4
  []
  [lower_arm]
    type = SubdomainBoundingBoxGenerator
    input = msh
    bottom_left = '0 0 0'
    top_right = '${length} ${arm} 0'
    block_id = 1
    block_name = lower_arm
  []
  [upper_arm]
    type = SubdomainBoundingBoxGenerator
    input = lower_arm
    bottom_left = '0 ${arm} 0'
    top_right = '${length} ${fparse 2 * arm} 0'
    block_id = 2
    block_name = upper_arm
  []
  [split]
    type = BreakMeshByBlockGenerator
    input = upper_arm
  []
  [lower_tip]
    type = ExtraNodesetGenerator
    input = split
    new_boundary = lower_tip
    coord = '0 0 0'
  []
  [upper_tip]
    type = ExtraNodesetGenerator
    input = lower_tip
    new_boundary = upper_tip
    coord = '0 ${fparse 2 * arm} 0'
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = FINITE
        decomposition_method = TaylorExpansion
        use_automatic_differentiation = true
        add_variables = true
        save_in = 'react_x react_y'
      []
    []
    [CohesiveZone]
      [interface]
        boundary = 'lower_arm_upper_arm'
      []
    []
  []
[]

[AuxVariables]
  [react_x]
  []
  [react_y]
  []
[]

[BCs]
  [open_up]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = upper_tip
    function = '0.05 * t'
  []
  [open_down]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = lower_tip
    function = '-0.05 * t'
  []
  [clamp_x]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0
  []
  [clamp_y]
    type = DirichletBC
    variable = disp_y
    boundary = right
    value = 0
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1.35e5
    poissons_ratio = 0.25
  []
  [stress]
    type = ADComputeFiniteStrainElasticStress
  []
  [czm_properties]
    type = GenericFunctionMaterial
    boundary = 'lower_arm_upper_arm'
    prop_names = 'N_strength S_strength GIc GIIc'
    prop_values = 'if(x<${precrack},1e-5,100) if(x<${precrack},1.5e-5,150)
                   if(x<${precrack},1e-12,0.28) if(x<${precrack},3e-12,0.8)'
  []
  [czm]
    type = BiLinearMixedModeTraction
    boundary = 'lower_arm_upper_arm'
    penalty_stiffness = 1e6
    GI_c = GIc
    GII_c = GIIc
    normal_strength = N_strength
    shear_strength = S_strength
    displacements = 'disp_x disp_y'
    eta = 2.2
  []
[]

[Postprocessors]
  [pad_reaction]
    type = NodalSum
    variable = react_y
    boundary = upper_tip
  []
  [P]
    type = ScalePostprocessor
    value = pad_reaction
    scaling_factor = 1
  []
  [opening]
    type = DifferencePostprocessor
    value1 = tip_up
    value2 = tip_down
  []
  [tip_up]
    type = PointValue
    variable = disp_y
    point = '0 ${fparse 2 * arm} 0'
  []
  [tip_down]
    type = PointValue
    variable = disp_y
    point = '0 0 0'
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
  solve_type = NEWTON
  line_search = none
  automatic_scaling = true
  dt = 0.25
  end_time = 40
  dtmin = 1e-4
  nl_max_its = 50
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
  exodus = true
[]
