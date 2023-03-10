# This is a verification problem for a circumferential crack in a solid cylinder.
# Crack radius to cylinder ratio: 0.2
# Crack radius to cylinder height: 0.1
# Tensile load 1MPa
# Analytical result: 1.596 (see [1]), MOOSE result 1.602 (Finite strain)
# [1]: Tran and Ginaut, 'Development of industrial applications of XFEM axisymmetric model for fracture mechanics', Eng. Frac. Mech., 82 (2012)

[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = 2drz_tran.e
  []
  # uniform_refine = 4
  coord_type = RZ
[]

[DomainIntegral]
  integrals = 'JIntegral InteractionIntegralKI'
  boundary = 1001
  radius_inner = '0.1 0.2 0.4'
  radius_outer = '0.1 0.2 0.4'
  crack_direction_method = CrackDirectionVector
  crack_direction_vector = '1 0 0'
  2d = true
  axis_2d = 2
  incremental = true
  symmetry_plane = 1

  youngs_modulus = 2e6
  poissons_ratio = 0.0
  block = '1'
[]

[Modules/TensorMechanics/Master]
  [master]
    strain = FINITE
    add_variables = true
    incremental = true
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress strain_xx strain_yy '
                      'elastic_strain_xx elastic_strain_yy'
    decomposition_method = EigenSolution
  []
[]

[BCs]
  [plane_y]
    type = DirichletBC
    variable = disp_y
    boundary = '5001'
    value = 0.0
  []
  [Pressure]
    [sigma_0]
      boundary = 6
      factor = 1
      function = -1
    []
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.0e6
    poissons_ratio = 0.0
  []
  [elastic_stress]
    type = ComputeFiniteStrainElasticStress
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  line_search = 'none'

  l_max_its = 50
  nl_max_its = 20
  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-8
  l_tol = 1e-6

  start_time = 0.0
  dt = 1.0
  num_steps = 1
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
  exodus = true
[]
