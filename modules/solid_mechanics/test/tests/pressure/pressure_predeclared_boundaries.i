[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  add_sideset_ids = 9999
  add_subdomain_ids = 8888
  [MeshGenerator]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
  []
  [rename]
    type = RenameBoundaryGenerator
    input = MeshGenerator
    old_boundary = 'right top'
    new_boundary = 'surround surround'
  []
[]

[MeshModifiers]
  [rename_sidesets]
    type = SidesetAroundSubdomainUpdater
    inner_subdomains = 0
    outer_subdomains = 8888
    update_sideset_name = 9999
    mask_side = 'surround'
    execute_on = 'TIMESTEP_END'
  []
[]


[Functions]
  [pressure]
    type = ParsedFunction
    expression = .01*t
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[Kernels]
  [SolidMechanics]
  []
[]

[BCs]
  [no_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [no_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [Pressure]
    [pressure]
      boundary = '9999'
      function = pressure
    []
  []
[]

[Materials]
  [Elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0 0.5e6'
  []
  [strain]
    type = ComputeSmallStrain
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  nl_abs_tol = 1e-10
  l_max_its = 20
  start_time = 0.0
  dt = 1.0
  num_steps = 10
  end_time = 2.0
[]
[Postprocessors]
  [disp_x_avg]
    type = ElementAverageValue
    variable = disp_x
    block = '0'
    execute_on = 'initial timestep_end'
  []
  [disp_y_avg]
    type = ElementAverageValue
    variable = disp_y
    block = '0'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  [out]
    type = Exodus
  []
[]
