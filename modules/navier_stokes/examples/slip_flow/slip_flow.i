[Mesh]
  [base_domains]
    type = FileMeshGenerator
    file = mesh.msh
  []
[]

[Variables]
  [vel]
    family = LAGRANGE_VEC
  []
  [p]
  []
[]

[Kernels]
  [mass]
    type = INSADConservativeMass
    variable = p
    velocity = vel
  []
  [mass_pspg]
    type = INSADMassPSPG
    variable = p
  []
  [momentum_advection]
    type = ADMomentumConservativeAdvection
    variable = vel
  []
  [momentum_viscous]
    type = INSADMomentumViscous
    variable = vel
    viscous_form = traction
  []
  [momentum_pressure]
    type = INSADMomentumPressure
    variable = vel
    pressure = p
    integrate_p_by_parts = true
  []
  [momentum_supg]
    type = INSADMomentumSUPG
    variable = vel
    velocity = vel
#    material_velocity = relative_velocity
  []
[]

[BCs]
  [no_slip]
    type = ADVectorFunctionDirichletBC
    variable = vel
    boundary = '0'
  []

  [in_flow]
    type = ADVectorFunctionDirichletBC
    variable = vel
    boundary = '4'
    set_y_comp = false
    set_z_comp = false
    function_x = (z+0.00204)/0.00204
  []

  [in_flow_mass]
    type = INSADMassFreeBC
    variable = p
    velocity = vel
    boundary = '4'
#    v_fn = (z+0.002)/0.002
  []

  [out_flow_1]
    type = INSADMassFreeBC
    variable = p
    velocity = vel
    boundary = '2'
  []

  [out_flow_2]
    type = DirichletBC
    variable = p
    boundary = '2'
    value = 0
  []

  [y_no_slip]
    type = ADVectorFunctionDirichletBC
    variable = vel
    boundary = '1 3'
    set_x_comp = false
    set_z_comp = false
  []

  [z_no_slip]
    type = ADVectorFunctionDirichletBC
    variable = vel
    boundary = '5'
    set_x_comp = false
    set_y_comp = false
  []

  [momentum_no_bc]
    type = INSADMomentumNoBCBC
    variable = vel
    pressure = p
    viscous_form = traction
    boundary = '2 4 100'
  []
[]

[Materials]
  [ins_mat]
    type = INSADStabilized3Eqn
    velocity = vel
    pressure = p
    temperature = 2500
  []
  [steel]
    type = ADGenericConstantMaterial
    prop_names = 'rho cp mu k'
    prop_values = '400 1 1 1'
    # prop_values = '1000 1 1 1' # Does not work
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
    petsc_options_value = 'lu       NONZERO               strumpack'
  []
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_converged_reason -ksp_converged_reason -options_left'
  solve_type = 'NEWTON'
  line_search = 'none'
  nl_max_its = 100
  l_max_its = 100
[]

[Outputs]
  [exodus]
    type = Exodus
    output_material_properties = true
  []
[]

[Debug]
  show_var_residual_norms = true
[]

