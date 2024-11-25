[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = 'mesh.inp'
    allow_renumbering = false
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    strain = finite
    planar_formulation = PLANE_STRAIN
  []
[]

[BCs]
  [x_bot]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
  []
  [y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e3
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9
  start_time = 0.0
  num_steps = 5
  dt = 1.0
  dtmin = 1.0
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Outputs]
  [ref]
    type = Exodus
  []
  [disp]
    type = Exodus
    use_displaced = true
  []
[]
