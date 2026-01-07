# 3D Double Cantiler beam example.
# Figure 9 in:
# @article{belytschko1999elastic,
#   title={Elastic crack growth in finite elements with minimal remeshing},
#   author={Belytschko, Ted and Black, Tom},
#   journal={International journal for numerical methods in engineering},
#   volume={45},
#   number={5},
#   pages={601--620},
#   year={1999},
#   publisher={Wiley Online Library}
# }

# to reproduce literature results the mesh must be refined with smaller q-integrals and smaller growth rates
# search comments for "to reproduce literature use"

L = 11.8
H = 3.94
W = 4
poissons = 0.3
youngs = 3e7

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[XFEM]
  geometric_cut_userobjects = 'cut_mesh'
  qrule = volfrac
  output_cut_plane = true
[]

[Mesh]
  # for cutter mesh with crack deviation angle = 5.71 deg
  [read_in_cutter_mesh]
    type = FileMeshGenerator
    file = double_cantilever_crack.xda
    save_with_name = cut_mesh
  []
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 30 # to reproduce literature use 240
    ny = 11 # to reproduce literature use 81
    nz = 4 # to reproduce literature use 10
    xmin = 0
    xmax = ${L}
    ymin = '-${fparse H/2}'
    ymax = '${fparse H/2}'
    zmin = '-${fparse W/2}'
    zmax = '${fparse W/2}'
    elem_type = HEX8
  []
  [bottom_left_elem]
    type = ParsedGenerateSideset
    input = gen
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

[Functions]
  [growth_func_v]
    type = ParsedFunction
    expression = 0.21 # to reproduce literature use 0.0125
  []
[]
[UserObjects]
  [cut_mesh]
    type = CrackMeshCut3DUserObject
    mesh_generator_name = 'cut_mesh'
    growth_dir_method = MAX_HOOP_STRESS
    size_control = 1
    n_step_growth = 1
    growth_rate = growth_func_v
    crack_front_nodes = '7 6 5 4'
  []
[]

[DomainIntegral]
  integrals = 'Jintegral InteractionIntegralKI InteractionIntegralKII'
  displacements = 'disp_x disp_y disp_z'
  crack_front_points_provider = cut_mesh
  number_points_from_provider = 4
  crack_direction_method = CurvedCrackFront
  radius_inner = 0.42 # to reproduce literature use 0.1
  radius_outer = 1.0 # to reproduce literature use 0.3
  poissons_ratio = ${poissons}
  youngs_modulus = ${youngs}
  incremental = false
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = SMALL
    incremental = false
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
  [right_z]
    type = DirichletBC
    boundary = 'right'
    variable = disp_z
    value = 0
  []
  [planar_z]
    type = DirichletBC
    boundary = 'front back'
    variable = disp_z
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
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'
  # petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -pc_factor_shift_type -pc_factor_shift_amount'
  # petsc_options_value = ' lu       superlu_dist                 NONZERO               1e-20'
  line_search = 'none'
  nl_abs_tol = 1e-7
  dt = 1.0
  end_time = 8 # to reproduce literature use 100
  max_xfem_update = 1
[]

[Outputs]
  # [xfemcutter]
  #   type = XFEMCutMeshOutput
  #   xfem_cutter_uo = cut_mesh
  # []
  # exodus = true
  csv = true
  execute_on=final
[]
