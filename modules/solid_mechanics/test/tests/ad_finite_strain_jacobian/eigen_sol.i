[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    xmax = 1.0
    ymax = 0.5
    zmax = 0.5
    nx = 2
    ny = 1
    nz = 1

    boundary_name_prefix = flexible
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    decomposition_method = EigenSolution # Using TaylorExpansion Works
    add_variables = true
    use_automatic_differentiation = true
  []
[]

[Materials]
  [elastic_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e+4
    poissons_ratio = 0.4
  []
  [stress]
    type = ADComputeFiniteStrainElasticStress
  []

[]

[BCs]
  [hold_x]
    type = ADDirichletBC
    boundary = 'flexible_bottom'
    variable = disp_x
    value = 0
  []
  [hold_y]
    type = ADDirichletBC
    boundary = 'flexible_bottom'
    variable = disp_y
    value = 0
  []
  [hold_z]
    type = ADDirichletBC
    boundary = 'flexible_bottom'
    variable = disp_z
    value = 0
  []
  [Pressure]
    [push]
      function = 't'
      boundary = 'flexible_left '
      use_automatic_differentiation = true
      use_displaced_mesh = false
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
  end_time = 2
  dt = 1
  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason '
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  # automatic_scaling = true
  line_search = 'none'
  nl_abs_tol = 1e-7
  nl_rel_tol = 1e-12
  nl_forced_its = 1

[]

[Outputs]

  exodus = true
  print_linear_residuals = false
[]
