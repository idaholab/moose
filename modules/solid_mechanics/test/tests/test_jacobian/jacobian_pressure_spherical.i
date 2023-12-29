[GlobalParams]
  displacements = 'disp_x'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
    xmin = 0.5
    xmax = 1.5
  []
[]

[Problem]
  coord_type = RSPHERICAL
[]

[Variables]
  [disp_x]
  []
[]


[Modules/TensorMechanics/Master]
  [all]
    incremental = false
    strain = SMALL
  []
[]

[BCs]
  [disp_x]
    type = Pressure
    variable = disp_x
    boundary = 'left right'
    factor = 1e8
  []
[]

[Materials]
  [stress]
    type = ComputeLinearElasticStress
  []

  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 3.7e11
    poissons_ratio = 0.345
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

  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'
  petsc_options = '-snes_test_jacobian -snes_test_jacobian_view'

  line_search = 'none'

  solve_type = NEWTON

  nl_rel_tol = 5e-6
  nl_abs_tol = 1e-10
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 50

  start_time = 0.0
  end_time = 1
  dt = 1
[]
