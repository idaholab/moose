mu=1.1
rho=1.1
advected_interp_method='average'
velocity_interp_method='rc'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 0
    xmax = 10
    ymin = -1
    ymax = 1
    zmin = -1
    zmax = 1
    nx = 20
    ny = 4
    nz = 4
  []
[]

[GlobalParams]
  # retain behavior at time of test creation
  two_term_boundary_expansion = false
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = u
    v = v
    w = w
    pressure = pressure
  []
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = 1
  []
  [v]
    type = INSFVVelocityVariable
    initial_condition = 1e-15
  []
  [w]
    type = INSFVVelocityVariable
    initial_condition = 1e-15
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = u
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    pressure = pressure
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = v
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = v
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    pressure = pressure
  []

  [w_advection]
    type = INSFVMomentumAdvection
    variable = v
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'z'
  []
  [w_viscosity]
    type = INSFVMomentumDiffusion
    variable = w
    mu = ${mu}
    momentum_component = 'z'
  []
  [w_pressure]
    type = INSFVMomentumPressure
    variable = w
    momentum_component = 'z'
    pressure = pressure
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = u
    function = '1'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = v
    function = '0'
  []
  [inlet-w]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = w
    function = '0'
  []
  [walls-u]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom front back'
    variable = u
    function = 0
  []
  [walls-v]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom front back'
    variable = v
    function = 0
  []
  [walls-w]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom front back'
    variable = w
    function = 0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = '0'
  []
[]

[Preconditioning]
  active = 'smp'
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -pc_factor_shift_type'
    petsc_options_value = 'lu       mumps                      NONZERO'
  []
  [fsp]
    type = FSP
    topsplit = 'nuv'
    [nuv]
      splitting = 'momentum_and_all mass'
      splitting_type = schur
      schur_type = full
      #schur_pre = S
      #petsc_options = '-dm_view'
      petsc_options_iname = '-pc_fieldsplit_schur_fact_type -pc_fieldsplit_schur_precondition'
      petsc_options_value = 'full selfp'
    []
    [momentum_and_all]
      vars = 'u v w'
      # petsc_options = '-ksp_monitor'
      petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_ksp_type'
      petsc_options_value = ' preonly  asm      jacobi          preonly'
      # lu        157
      # jacobi    81
      # bjacobi   90
      # sor       91
      # eisenstat 89
      # icc (not parallel)
      # ilu      179
      # asm       81
      # gasm      97
      # ksp      419
      # cholesky  no CV

      # petsc_options_iname = '-ksp_type -pc_type -ksp_gmres_restart -pc_factor_mat_solver_package'
      # petsc_options_value = 'preonly lu       30                 strumpack'
      # strumpack lu solve: 117s / 115s preonly?
      # asm lu strumpack 190s
    []
    [mass]
      vars = 'pressure'
      #petsc_options = '-pc_svd_monitor'
      # petsc_options = '-ksp_monitor'
      petsc_options_iname = '-ksp_type -pc_type -pc_hypre_type'
      petsc_options_value = ' preonly   hypre  boomeramg'
      #full = true
    []
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  line_search = 'none'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
  csv = true
  [dof]
    type = DOFMap
    execute_on = 'initial'
  []
[]
