# Center inclined crack. To run convergence study, run angles for theta=0-90 and refinement mesh_h=201,401,801
#
# @article{moes1999finite,
#   title={A finite element method for crack growth without remeshing},
#   author={Mo{\"e}s, Nicolas and Dolbow, John and Belytschko, Ted},
#   journal={International journal for numerical methods in engineering},
#   volume={46},
#   number={1},
#   pages={131--150},
#   year={1999},
#   publisher={Wiley Online Library}
# }
# @article{richardson2011xfem,
#   title={An XFEM method for modeling geometrically elaborate crack propagation in brittle materials},
#   author={Richardson, Casey L and Hegemann, Jan and Sifakis, Eftychios and Hellrung, Jeffrey and Teran, Joseph M},
#   journal={International Journal for Numerical Methods in Engineering},
#   volume={88},
#   number={10},
#   pages={1042--1065},
#   year={2011},
#   publisher={Wiley Online Library}
# }
H = 40
W = 40
a = 1
theta = 20  #measured from x-axis
poissons = 0.3
youngs = 30e6
stress_load = 10000
mesh_h=201

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]


[Mesh]
  [cutter_mesh]
    type = PolyLineMeshGenerator
    points = '-${fparse a*cos(theta*pi/180)} -${fparse a*sin(theta*pi/180)} 0
              ${fparse a*cos(theta*pi/180)} ${fparse a*sin(theta*pi/180)} 0'
    loop = false
    num_edges_between_points = 2
    save_with_name = cut_mesh
  []
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = ${mesh_h}
    ny = ${mesh_h}
    xmin = -${fparse W/2}
    xmax = ${fparse W/2}
    ymin = -${fparse H/2}
    ymax = ${fparse H/2}
    elem_type = QUAD4
  []
  [center_block]
    type = SubdomainBoundingBoxGenerator
    input = gen
    block_id = 10
    bottom_left = '-${fparse 1.5*a} -${fparse 1.5*a} 0'
    top_right = '${fparse 1.5*a} ${fparse 1.5*a} 0'
  []
  [center_left_node]
    type = ExtraNodesetGenerator
    coord = '-${fparse W/2} 0 0'
    input = gen
    new_boundary = 'center_left_node'
    use_closest_node = true
  []
  [center_right_node]
    type = ExtraNodesetGenerator
    coord = '${fparse W/2} 0 0'
    input = center_left_node
    new_boundary = 'center_right_node'
    use_closest_node = true
  []
  final_generator = 'center_right_node'
[]

#### - adaptivity causes segfault, see #31714
# [AuxVariables]
#   [constant_refine]
#     initial_condition = 2
#     order = CONSTANT
#     family = MONOMIAL
#     block = 10
#   []
# []
# [Adaptivity]
#   initial_marker = constant_refine
#   max_h_level = 2
#   initial_steps = 2
# []

[XFEM]
  geometric_cut_userobjects = 'cut_mesh'
  qrule = volfrac
  output_cut_plane = true
[]

[UserObjects]
  [cut_mesh]
    type = MeshCut2DFractureUserObject
    mesh_generator_name = 'cut_mesh'
    growth_increment = 0.05
    ki_vectorpostprocessor = "II_KI_1"
    kii_vectorpostprocessor = "II_KII_1"
    k_critical = 1000 # big, don't want to grow
  []
[]

[DomainIntegral]
  integrals = 'Jintegral InteractionIntegralKI InteractionIntegralKII'
  displacements = 'disp_x disp_y'
  crack_front_points_provider = cut_mesh
  2d = true
  number_points_from_provider = 2
  crack_direction_method = CurvedCrackFront
  radius_inner = '0.2'
  radius_outer = '0.8'
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

[Postprocessors]
  [theta]
    type = ConstantPostprocessor
    value = ${theta}
  []
  [stress_load]
    type = ConstantPostprocessor
    value = ${stress_load}
  []
  [ki_analytic]
    type = ParsedPostprocessor
    expression = '(${fparse stress_load*sqrt(pi*a)*cos(theta*pi/180)*cos(theta*pi/180)})'
  []
  [kii_analytic]
    type = ParsedPostprocessor
    expression = '(${fparse stress_load*sqrt(pi*a)*cos(theta*pi/180)*sin(theta*pi/180)})'
  []
[]

[BCs]
  [left_x]
    type = DirichletBC
    boundary = 'center_left_node'
    variable = disp_x
    value = 0
  []
  [left_y]
    type = DirichletBC
    boundary = 'center_left_node'
    variable = disp_y
    value = 0
  []
  [right_y]
    type = DirichletBC
    boundary = 'center_right_node'
    variable = disp_y
    value = 0
  []
  [bottom_load]
    type = NeumannBC
    boundary = 'bottom'
    variable = disp_y
    value = -${stress_load}
  []
  [top_load]
    type = NeumannBC
    boundary = 'top'
    variable = disp_y
    value = ${stress_load}
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
  start_time = 0.0
  dt = 1.0
  end_time = 1
  max_xfem_update = 0
[]

[Outputs]
  csv = true
  # uncomment for convergence study
  # file_base = inclined_crack/results_theta_${theta}_h_${mesh_h}
[]
