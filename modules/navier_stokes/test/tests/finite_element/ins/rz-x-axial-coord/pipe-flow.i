# 2d siumulation of a water through a pipe.
mu=1e-3 # Nsm^-2
rho=997.0 # kgm^-3
Re=1000.0
pipe_length=1 # m
pipe_radius=0.1 # m
u_inlet=${fparse (mu * Re)/(2 * pipe_radius * rho)} # ms^-1

[GlobalParams]
  integrate_p_by_parts = false
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${pipe_length}
    ymin = 0
    ymax = ${pipe_radius}
    nx = 50
    ny = 5
  []
[]

[Problem]
  coord_type = 'RZ'
  rz_coord_axis = x
[]

[Variables]
  [velocity]
    family = LAGRANGE_VEC
  []
  [p][]
[]

[ICs]
  [velocity]
    type = VectorConstantIC
    x_value = 1e-15
    y_value = 1e-15
    variable = velocity
  []
[]

[Kernels]
  [mass]
    type = INSADMass
    variable = p
  []
  [mass_pspg]
    type = INSADMassPSPG
    variable = p
  []
  [momentum_convection]
    type = INSADMomentumAdvection
    variable = velocity
  []
  [momentum_viscous]
    type = INSADMomentumViscous
    variable = velocity
  []
  [momentum_pressure]
    type = INSADMomentumPressure
    variable = velocity
    pressure = p
  []
  [momentum_supg]
    type = INSADMomentumSUPG
    variable = velocity
    velocity = velocity
  []
[]

[BCs]
  [inlet]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'left'
    function_x = ${u_inlet}
    function_y = 0
  [../]
  [wall]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'top'
    function_x = 0
    function_y = 0
  []
  [axis]
    type = ADVectorFunctionDirichletBC
    variable = velocity
    boundary = 'bottom'
    set_x_comp = false
    function_y = 0
  []
  # pressure is not integrated by parts so we cannot remove the nullspace through a natural condition
  [p_corner]
    type = DirichletBC
    boundary = 'right'
    value = 0
    variable = p
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
  [ins_mat]
    type = INSADTauMaterial
    velocity = velocity
    pressure = p
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -sub_pc_factor_levels -ksp_gmres_restart'
  petsc_options_value = 'asm      6                     200'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
  l_max_its = 200
  line_search = 'none'
[]

[Outputs]
  exodus = true
[]
