[GlobalParams]
  order = SECOND
  family = LAGRANGE
  displacements = 'disp_x disp_y'
  large_kinematics = true
  use_displaced_mesh = true
[]

[Problem]
  type = ReferenceResidualProblem
  reference_vector = 'ref'
  extra_tag_vectors = 'ref'
  material_coverage_check = false
  kernel_coverage_check = false
[]

[Mesh]
  use_displaced_mesh = true
  displacements = 'disp_x disp_y'
  [block_B]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 5
    ny = 5
    elem_type = QUAD8
    boundary_id_offset = 4
    subdomain_ids = 2
    subdomain_name = B
    boundary_name_prefix = b
  []
[]

[Materials]
  [elasticity_tensor_B]
    type = ComputeElasticityTensor
    block = B
    fill_method = symmetric_isotropic
    C_ijkl = '1 0'
    #youngs_modulus = 1 #2.0e11
    #poissons_ratio = 0 #0.345
  []
  [stress_B]
    type = ComputeFiniteStrainElasticStress
    block = B
  []
  [stress_wrapped_B]
    type = ComputeLagrangianWrappedStress
    block = B
    objective_rate = jaumann
  []
[]

[MeshModifiers]
  [nodal_values]
    type = UndisplacedMeshUpdater
    block = B
    execute_on = 'TIMESTEP_END'
    variables = 'disp_x disp_y'
    threshold = 0.5
    criterion_type = 'Above'
    criterion_variable = phi
    use_displaced_mesh = false
    execution_order_group = -1
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [block_B]
        block = B
        strain = SMALL
        formulation = TOTAL
        new_system = true
        generate_output = 'stress_xx
                           stress_yy
                           stress_xy
                           stress_zz
                           strain_xx
                           strain_yy
                           strain_xy
                           strain_zz'
      []
    []
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[Functions]
  [pressure_func]
    type = ParsedFunction
    expression = 'if (t < 3, .3, 0)'
  []
[]

[AuxVariables]
  [phi]
  []
[]

[Kernels]
  [diff_x]
    type = Diffusion
    variable = disp_x
  []
  [diff_y]
    type = Diffusion
    variable = disp_y
  []
[]

[AuxKernels]
  [switch_block]
    type = ParsedAux
    expression = "if(t=3, 1, 0)"
    use_xyzt = true
    variable = phi
    use_displaced_mesh = false
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[BCs]
  [no_x]
    type = DirichletBC
    boundary = 7
    value = 0
    variable = disp_x
  []
  [no_y]
    type = DirichletBC
    boundary = 4
    value = 0
    variable = disp_y
  []
  [Pressure]
    [outer_pressure]
      boundary = 5
      function = pressure_func
      factor = 1
    []
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -snes_type'
  petsc_options_value = 'lu       superlu_dist vinewtonrsls'
  automatic_scaling = true
  compute_scaling_once = false
  line_search = 'none'

  start_time = 0
  dt = 1
  end_time = 3 #20
  verbose = true

  l_max_its = 100
  l_tol = 8e-3
  nl_max_its = 40
  nl_rel_tol = 1e-4
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
[]
