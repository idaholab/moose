# The DCB of dcb_arclength.i under plain load control: the same peel forces
# are ramped directly with time, load factor = t, and every step is an
# ordinary Newton solve. At the first serration of the sawtooth there is no
# equilibrium at a higher load, so Newton diverges, the TimeStepper cuts back
# to dtmin, and the run aborts. The arc-length run rides the sawtooth on.

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
      []
    []
    [CohesiveZone]
      [interface]
        boundary = 'lower_arm_upper_arm'
      []
    []
  []
[]

[DiracKernels]
  # the continuation load: an opening force couple peeling the arm tips apart
  [peel_up]
    type = FunctionDiracSource
    variable = disp_y
    point = '0 ${fparse 2 * arm} 0'
    function = '10 * t'
  []
  [peel_down]
    type = FunctionDiracSource
    variable = disp_y
    point = '0 0 0'
    function = '-10 * t'
  []
[]

[BCs]
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
  # strengths sized so the cohesive zone spans one or two elements, which is
  # what makes the crack advance element by element and the curve serrated
  # the pre-crack is carved from the cohesive properties: ahead of it the
  # interface carries the real strengths and toughnesses, behind it next to
  # nothing, so the first load increment fails it into free faces
  [czm_properties]
    type = GenericFunctionMaterial
    boundary = 'lower_arm_upper_arm'
    prop_names = 'N_strength S_strength GIc GIIc'
    # the pre-crack strip fails into free faces during the first step: its
    # softening window is thinner than any numerical opening
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
  [P]
    type = FunctionValuePostprocessor
    function = '10 * t'
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
  dt = 0.05
  end_time = 6
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
