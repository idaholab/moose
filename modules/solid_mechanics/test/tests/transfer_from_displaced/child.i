[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    ymin = 0
    xmax = 0.2
    ymax = 0.5
    nx = 5
    ny = 15
    elem_type = QUAD4
  []
[]

[Variables]
  [disp_x][]
  [disp_y][]
[]

[AuxVariables]
  [solid_indicator]
    [AuxKernel]
      type = ConstantAux
      variable = solid_indicator
      value = 0.0
      boundary = 'left right top'
      execute_on = 'initial timestep_end'
    []
    initial_condition = 1.0
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = SMALL
    incremental = true
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 10000.0
    poissons_ratio = 0.3
    use_displaced_mesh = true
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
[]

[BCs]
  [move_bottom_x]
    type = FunctionDirichletBC
    boundary = bottom
    variable = disp_x
    function = 't'
  []
  [move_bottom_y]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = '0'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  solve_type = 'NEWTON'
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -pc_factor_shift_type'
  petsc_options_value = 'lu       superlu_dist               NONZERO'
  nl_max_its = 40
  l_max_its = 15
  line_search = 'none'
  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-4
  automatic_scaling = true
[]
