[GlobalParams]
  prevent_boundary_ids_overlap = false
[]

[Mesh]
  [region_2_gen]
      type = CartesianMeshGenerator
      dim = 2
      dx = '0.065 0.13 0.305 0.17 0.196'
      ix = '  2    2     2    2     2'
      dy = '0.85438 '
      iy = '6'
      subdomain_id = '68 68 68 68 68'
  []
  [region_2_move]
      type = TransformGenerator
      transform = TRANSLATE
      vector_value = '1.2 1.551 0'
      input = region_2_gen
  []
  [region_3_gen]
      type = CartesianMeshGenerator
      dim = 2
      dx = '0.24 0.24 0.24 0.24 0.24'
      ix = ' 2   2   2   2   2'
      dy = '0.744166666666666 0.744166666666667 0.744166666666667'
      iy = ' 2 2 2'
      subdomain_id = '56 57 58 59 60
                      51 52 53 54 55
                      46 47 48 49 50'
  []
  [region_3_move]
      type = TransformGenerator
      transform = TRANSLATE
      vector_value = '0 2.40538 0'
      input = region_3_gen
  []
  [region_1_gen]
      type = GeneratedMeshGenerator
      dim = 2
      nx = 10
      ny = 6
      xmin = 0
      xmax = 0.26
      ymin = 1.551
      ymax = 1.851
      subdomain_ids = '62 62 62 62 62 62 62 62 62 62
                       62 62 62 62 62 62 62 62 62 62
                       62 62 62 62 62 62 62 62 62 62
                       62 62 62 62 62 62 62 62 62 62
                       62 62 62 62 62 62 62 62 62 62
                       62 62 62 62 62 62 62 62 62 62'
  []
  [region_1_extend_1]
      type = FillBetweenSidesetsGenerator
      input_mesh_1 = 'region_3_move'
      input_mesh_2 = 'region_1_gen'
      boundary_1 = '0'
      boundary_2 = '2'
      num_layers = 6
      block_id= 61
      use_quad_elements = true
      keep_inputs = true
      begin_side_boundary_id = '3'
      end_side_boundary_id = '1'
  []
  [region_1_extend_2]
      type = FillBetweenSidesetsGenerator
      input_mesh_1 = 'region_2_move'
      input_mesh_2 = 'region_1_gen'
      boundary_1 = 3
      boundary_2 = 1
      num_layers = 6
      block_id= 69
      use_quad_elements = true
      keep_inputs = false
      begin_side_boundary_id = '0'
      end_side_boundary_id = '3'
      input_boundary_1_id = '1'
      input_boundary_2_id = '3'
  []
  [region_2_2_gen]
      type = CartesianMeshGenerator
      dim = 2
      dx = '0.065 0.13 0.305 0.17 0.196'
      ix = '  2    2     2    2     2'
      dy = '0.85438 '
      iy = '6'
      subdomain_id = '68 68 68 68 68'
  []
  [region_2_2_move]
      type = TransformGenerator
      transform = TRANSLATE
      vector_value = '1.2 1.551 0'
      input = region_2_2_gen
  []
  [region_6_gen]
      type = CartesianMeshGenerator
      dim = 2
      dx = '0.26 0.94 0.065 0.13 0.305 0.17 0.196'
      ix = '10  6     2    2     2    2     2'
      dy = '0.584 0.967'
      iy = '  4    6'
      subdomain_id = '62 72 72 72 72 72 72
                      62 70 71 71 71 71 71'
  []
  [stitch_1_2_6]
      type = StitchedMeshGenerator
      inputs = 'region_1_extend_1 region_1_extend_2 region_2_2_move region_6_gen'
      stitch_boundaries_pairs = '1   3;
                                 1   3;
                                 0   2'
  []
  [rename_boundary_stitch_1_2_6]
      type = RenameBoundaryGenerator
      input = stitch_1_2_6
      old_boundary = '1'
      new_boundary = '2'
  []
  [region_4_gen]
      type = CartesianMeshGenerator
      dim = 2
      dx = '0.065 0.13'
      ix = '  2    2  '
      dy = '0.744166666666666 0.744166666666667 0.744166666666667'
      iy = ' 2 2 2'
      subdomain_id = '78 92
                      78 91
                      78 90'
  []
  [region_4_move]
      type = TransformGenerator
      transform = TRANSLATE
      vector_value = '1.2 2.40538 0'
      input = region_4_gen
  []
  [region_5_gen]
      type = CartesianMeshGenerator
      dim = 2
      dx = '0.17 0.196'
      ix = '2     2'
      dy = '0.39  1.8425'
      iy = '2 4'
      subdomain_id = '100 104
                      100 104'
  []
  [region_5_move]
      type = TransformGenerator
      transform = TRANSLATE
      vector_value = '1.7 2.40538 0'
      input = region_5_gen
  []
  [region_5_extend]
      type = FillBetweenSidesetsGenerator
      input_mesh_1 = 'region_4_move'
      input_mesh_2 = 'region_5_move'
      boundary_1 = 1
      boundary_2 = 3
      num_layers = 2
      block_id= 96
      use_quad_elements = true
      keep_inputs = true
      begin_side_boundary_id = '0'
      end_side_boundary_id = '2'
  []
  [rename_boundary_region_5]
      type = RenameBoundaryGenerator
      input = region_5_extend
      old_boundary = '0'
      new_boundary = '3'
  []
  [stitch_1_2_6_5]
      type = StitchedMeshGenerator
      inputs = 'rename_boundary_stitch_1_2_6 rename_boundary_region_5'
      stitch_boundaries_pairs = '2     3;'
  []
  [region_7_gen]
      type = CartesianMeshGenerator
      dim = 2
      dx = '0.24 0.24 0.24 0.24 0.24 0.065 0.13 0.305 0.17 0.196'
      ix = '  2    2    2    2    2      2    2     2    2     2'
      dy = '0.744166666666667 0.744166666666667 0.744166666666667 0.744166666666667
            0.744166666666667 0.744166666666667 0.744166666666666 0.744166666666666
            0.744166666666666 0.458 0.86002'
      iy = '2 2 2 2 2 2 2 2 2 2 4'
      subdomain_id = '41 42 43 44 45 77 89 95 99 103
                      36 37 38 39 40 77 88 95 99 103
                      31 32 33 34 35 77 87 95 99 103
                      26 27 28 29 30 76 86 94 98 102
                      21 22 23 24 25 76 85 94 98 102
                      16 17 18 19 20 76 84 94 98 102
                      11 12 13 14 15 75 83 93 97 101
                       6  7  8  9 10 75 82 93 97 101
                       1  2  3  4  5 75 81 93 97 101
                      67 67 67 67 67 74 80 65 65  66
                      63 63 63 63 63 73 79 64 64  64'
  []
  [region_7_move]
      type = TransformGenerator
      transform = TRANSLATE
      vector_value = '0.0 4.63788 0'
      input = region_7_gen
  []
  [stitch]
    type = StitchedMeshGenerator
    inputs = 'stitch_1_2_6_5 region_7_move'
    stitch_boundaries_pairs = '2 0'
  []
  [rename_boundary_1]
    type = BoundaryDeletionGenerator
    input = stitch
    boundary_names = '0 1 2 3'
  []
  [rename_boundary_2]
      type = SideSetsFromPointsGenerator
      input = rename_boundary_1
      new_boundary = '2 4 1 3'
      points = '1.2 0. 0.
                2.066 1.551 0.
                1.2 12.6534 0.
                0. 1.551 0.'
  []
  [rename_boundary_3]
      type = RenameBoundaryGenerator
      input = rename_boundary_2
      new_boundary = 'rbottom ssright rtop ssleft'
      old_boundary = '2 4 1 3'
  []
[rename_blocks]
      type = RenameBlockGenerator
      input = rename_boundary_3
      old_block = '1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
                  21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40
                  41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60
                  61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80
                  81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 97 98 99 100
                  101 102 103 104'
      new_block = 'pbedfuel001 pbedfuel002 pbedfuel003 pbedfuel004 pbedfuel005
                   pbedfuel006 pbedfuel007 pbedfuel008 pbedfuel009 pbedfuel010
                   pbedfuel011 pbedfuel012 pbedfuel013 pbedfuel014 pbedfuel015
                   pbedfuel016 pbedfuel017 pbedfuel018 pbedfuel019 pbedfuel020
                   pbedfuel021 pbedfuel022 pbedfuel023 pbedfuel024 pbedfuel025
                   pbedfuel026 pbedfuel027 pbedfuel028 pbedfuel029 pbedfuel030
                   pbedfuel031 pbedfuel032 pbedfuel033 pbedfuel034 pbedfuel035
                   pbedfuel036 pbedfuel037 pbedfuel038 pbedfuel039 pbedfuel040
                   pbedfuel041 pbedfuel042 pbedfuel043 pbedfuel044 pbedfuel045
                   pbedfuel046 pbedfuel047 pbedfuel048 pbedfuel049 pbedfuel050
                   pbedfuel051 pbedfuel052 pbedfuel053 pbedfuel054 pbedfuel055
                   pbedfuel056 pbedfuel057 pbedfuel058 pbedfuel059 pbedfuel060
                   consfuel061 dischfuel062 upref063 upref064 upref065 upref066
                   upcvt067 lwref068 outch069 lwrpln070 htleg071 lwref072 buffr073
                   buffr074 buffr075 buffr076 buffr077 buffr078 crds079 crds080
                   crds081 crds082 crds083 crds084 crds085 crds086 crds087 crds088
                   crds089 crds090 crds091 crds092 radrf093 radrf094 radrf095 radrf096
                   risr097 risr098 risr099 risr100 radrf101 radrf102 radrf103 radrf104'
  []
[]

[Variables]
  [T_solid]
    type = MooseVariableFVReal
    initial_condition = 100
  []
[]

[FVKernels]
  [energy_storage]
    type = FVTimeKernel
    variable = T_solid
  []
  [solid_energy_diffusion_core]
    type = FVAnisotropicDiffusion
    variable = T_solid
    coeff = 'effective_thermal_conductivity'
  []
[]

[FVBCs]
  [side_set_bc1]
    type = FVDirichletBC
    variable = T_solid
    value = '300'
    boundary = 'rtop'
  []
  [side_set_bc2]
    type = FVDirichletBC
    variable = T_solid
    value = '600'
    boundary = 'rbottom'
  []
[]


[Materials]
  [all_channels_porosity]
    type = ADGenericFunctorMaterial
    prop_names = 'porosity'
    prop_values = 0.5
  []
  [solid_blocks_full_density_graphite]
    type = ADGenericFunctorMaterial
    prop_names = 'rho_s cp_s k_s '
    prop_values = '1.0 2.0 3.0'
  []
  [effective_solid_thermal_conductivity_solid_only]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'effective_thermal_conductivity'
    prop_values = 'k_s k_s k_s'
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
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_factor_shift_type'
  petsc_options_value = 'lu        100                NONZERO'

  # Tolerances.
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-9
  line_search = none
  nl_max_its = 15

  [TimeStepper]
    type = IterationAdaptiveDT
    dt                 = 0.05
    cutback_factor     = 0.5
    growth_factor      = 2.00
    optimal_iterations = 6
  []

  # Steady state detection.
  steady_state_detection = true
  steady_state_tolerance = 1e-13

  abort_on_solve_fail = true
  num_steps = 1
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  print_linear_converged_reason = false
  print_nonlinear_converged_reason = false
[]
