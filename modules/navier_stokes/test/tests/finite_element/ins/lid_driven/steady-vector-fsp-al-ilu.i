rho=1
mu=1e-2
U=1
l=1
prefactor=${fparse 1/(l/2)^2}
n=375
gamma=1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${l}
    ymin = 0
    ymax = ${l}
    nx = ${n}
    ny = ${n}
    elem_type = QUAD4
  []
  second_order = true
[]

[Variables]
  [vel]
    order = SECOND
    family = LAGRANGE_VEC
  []
  [p]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [mass]
    type = INSADMass
    variable = p
  []
  [mass_kernel]
    type = MassMatrix
    variable = p
    matrix_tags = 'mass'
    density = ${fparse -1/(gamma + mu)}
  []
  [momentum_advection]
    type = INSADMomentumAdvection
    variable = vel
  []
  [momentum_viscous]
    type = INSADMomentumViscous
    variable = vel
  []
  [momentum_pressure]
    type = INSADMomentumPressure
    variable = vel
    pressure = p
    integrate_p_by_parts = true
  []
  [momentum_graddiv]
    type = INSADMomentumGradDiv
    variable = vel
    gamma = ${gamma}
  []
[]

[BCs]
  [no_slip]
    type = ADVectorFunctionDirichletBC
    variable = vel
    boundary = 'bottom right left'
    preset = true
  []
  [lid]
    type = ADVectorFunctionDirichletBC
    variable = vel
    boundary = 'top'
    function_x = 'lid_function'
    preset = true
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
  [insad]
    type = INSADTauMaterial
    velocity = vel
    pressure = p
  []
[]

[Functions]
  [lid_function]
    # We pick a function that is exactly represented in the velocity
    # space so that the Dirichlet conditions are the same regardless
    # of the mesh spacing.
    type = ParsedFunction
    expression = '${prefactor}*${U}*x*(${l}-x)'
  []
[]

[Problem]
  type = NavierStokesProblem
  extra_tag_matrices = 'mass'
  mass_matrix = 'mass'
  use_pressure_mass_matrix = true
  schur_fs_index = '1'
[]

[Preconditioning]
  [FSP]
    type = FSP
    topsplit = 'by_diri_others'
    [by_diri_others]
      splitting = 'diri others'
      splitting_type  = additive
      petsc_options_iname = '-ksp_type'
      petsc_options_value = 'preonly'
    []
      [diri]
        sides = 'left right top bottom'
        vars = 'vel'
        petsc_options_iname = '-pc_type'
        petsc_options_value = 'none'
      []
      [others]
        splitting = 'u p'
        splitting_type  = schur
        petsc_options = '-ksp_converged_reason'
        petsc_options_iname = '-pc_fieldsplit_schur_fact_type  -pc_fieldsplit_schur_precondition -ksp_gmres_restart -ksp_rtol -ksp_type -ksp_atol'
        petsc_options_value = 'full                            self                              300                1e-5      fgmres    1e-9'
        unside_by_var_boundary_name = 'left top right bottom'
        unside_by_var_var_name =      'vel  vel vel   vel'
      []
        [u]
          vars = 'vel'
          unside_by_var_boundary_name = 'left top right bottom'
          unside_by_var_var_name =      'vel  vel vel   vel'
          petsc_options = '-ksp_converged_reason'
          petsc_options_iname = '-pc_type  -ksp_type -ksp_rtol -ksp_gmres_restart -ksp_pc_side -pc_factor_mat_solver_type'
          petsc_options_value = 'ilu       gmres     1e-2      300                right        strumpack'
        []
        [p]
          vars = 'p'
          petsc_options = '-ksp_converged_reason'
          petsc_options_iname = '-ksp_type -ksp_gmres_restart -ksp_rtol -pc_type -ksp_pc_side'
          petsc_options_value = 'fgmres    300                1e-2      ilu      right'
        []
  []
[]

[Postprocessors]
  [pavg]
    type = ElementAverageValue
    variable = p
  []
[]

[UserObjects]
  [set_pressure]
    type = NSPressurePin
    pin_type = 'average'
    variable = p
    pressure_average = 'pavg'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_rel_tol = 2e-4
[]

[Outputs]
  print_linear_residuals = true
[]
