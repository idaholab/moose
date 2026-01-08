# 2D Double Cantiler beam example.
# Figure 9 in:
# Figure 9 in: Belytschko and Black (1999)
# https://doi.org/10.1002/(SICI)1097-0207(19990620)45:5<601::AID-NME598>3.0.CO;2-S

# to reproduce literature results the mesh must be refined with smaller q-integrals and smaller growth rates
# search comments for "to reproduce literature use"

L = 11.8
W = 3.94
a = 3.94
da = 0.3
dtheta = 5.71 # 1.43, 2.86, 5.71
poissons = 0.3
youngs = 3e7

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[XFEM]
  geometric_cut_userobjects = 'cut_mesh'
  qrule = volfrac
  output_cut_plane = true
[]

[Mesh]
  [cutter_mesh]
    type = PolyLineMeshGenerator
    points = '-0.01 0 0
              ${a} 0 0
              ${fparse a+da*cos(dtheta*pi/180)} ${fparse da*sin(dtheta*pi/180)} 0'
    loop = false
    num_edges_between_points = 2
    save_with_name = cut_mesh
  []
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 30 # to reproduce literature use 240
    ny = 11 # to reproduce literature use 81
    xmin = 0
    xmax = ${L}
    ymin = '-${fparse W/2}'
    ymax = '${fparse W/2}'
    elem_type = QUAD4
  []
  [bottom_left_node]
    type = ExtraNodesetGenerator
    coord = '0 -${fparse W/2} 0'
    input = gen
    new_boundary = 'bottom_left_node'
    use_closest_node = true
  []
  [top_left_node]
    type = ExtraNodesetGenerator
    coord = '0 ${fparse W/2} 0'
    input = bottom_left_node
    new_boundary = 'top_left_node'
    use_closest_node = true
  []
  [bottom_left_elem]
    type = ParsedGenerateSideset
    input = top_left_node
    combinatorial_geometry = 'x < 0.21'
    normal = '0 -1 0'
    include_only_external_sides=true
    new_sideset_name = bottom_left_elem
  []
  [top_left_elem]
    type = ParsedGenerateSideset
    input = bottom_left_elem
    combinatorial_geometry = 'x < 0.21'
    normal = '0 1 0'
    include_only_external_sides=true
    new_sideset_name = top_left_elem
  []
  final_generator = 'top_left_elem'
[]

[UserObjects]
  [cut_mesh]
    type = MeshCut2DFractureUserObject
    mesh_generator_name = 'cut_mesh'
    growth_increment = 0.21 #to reproduce literature use 0.0125
    ki_vectorpostprocessor = "II_KI_1"
    kii_vectorpostprocessor = "II_KII_1"
    k_critical = 0
  []
[]

[DomainIntegral]
  integrals = 'Jintegral InteractionIntegralKI InteractionIntegralKII'
  displacements = 'disp_x disp_y'
  crack_front_points_provider = cut_mesh
  2d = true
  number_points_from_provider = 1
  crack_direction_method = CurvedCrackFront
  radius_inner = 0.42 # to reproduce literature use 0.1
  radius_outer = 1.0 # to reproduce literature use 0.3
  poissons_ratio = ${poissons}
  youngs_modulus = ${youngs}
  block = 0
  incremental = false
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = SMALL
    incremental = false
    planar_formulation = plane_strain
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress max_principal_stress'
    add_variables = true
  []
[]

[BCs]
  [right_x]
    type = DirichletBC
    boundary = 'right'
    variable = disp_x
    value = 0
  []
  [right_y]
    type = DirichletBC
    boundary = 'right'
    variable = disp_y
    value = 0
  []
  [bottom_left_elem_y]
    type = NeumannBC
    boundary = 'bottom_left_elem'
    variable = disp_y
    value = -1000
  []
  [top_left_elem_y]
    type = NeumannBC
    boundary = 'top_left_elem'
    variable = disp_y
    value = 1000
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = ${youngs}
    poissons_ratio = ${poissons}
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = ' lu       superlu_dist                 NONZERO               1e-20'
  line_search = 'none'
  nl_abs_tol = 1e-7
  dt = 1.0
  end_time = 8 #to reproduce literature use 120
  max_xfem_update = 1
[]

[Outputs]
  # [xfemcutter]
  #   type = XFEMCutMeshOutput
  #   xfem_cutter_uo = cut_mesh
  # []
  # exodus = true
  csv = true
  execute_on = final
[]
