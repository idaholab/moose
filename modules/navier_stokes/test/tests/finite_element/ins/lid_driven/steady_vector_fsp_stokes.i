rho=1
mu=1
U=1
l=1
prefactor=${fparse 1/(l/2)^2}
n=8

[Mesh]
  [gen]
    type = DistributedRectilinearMeshGenerator
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
  parallel_type = distributed
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
[]

[BCs]
  [no_slip]
    type = ADVectorFunctionDirichletBC
    variable = vel
    boundary = 'bottom right left'
  []
  [lid]
    type = ADVectorFunctionDirichletBC
    variable = vel
    boundary = 'top'
    function_x = 'lid_function'
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
  [insad]
    type = INSADMaterial
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
  mass_matrix = 'mass'
  extra_tag_matrices = 'mass'
  use_pressure_mass_matrix = true
[]

[Preconditioning]
  [FSP]
    type = FSP
    topsplit = 'up'
    [up]
      splitting = 'u p'
      splitting_type  = schur
      petsc_options_iname = '-pc_fieldsplit_schur_fact_type  -pc_fieldsplit_schur_precondition -ksp_gmres_restart -ksp_type -ksp_pc_side -ksp_rtol'
      petsc_options_value = 'full                            self                              300                fgmres    right        1e-4'
    []
      [u]
        vars = 'vel'
        # petsc_options = '-ksp_converged_reason'
        petsc_options_iname = '-pc_type -pc_hypre_type -ksp_type -ksp_rtol -ksp_gmres_restart -ksp_pc_side'
        petsc_options_value = 'hypre    boomeramg      gmres     1e-2      300                right'
      []
      [p]
        vars = 'p'
        petsc_options = '-ksp_converged_reason'
        petsc_options_iname = '-ksp_type -ksp_gmres_restart -ksp_rtol -pc_type -ksp_pc_side -pc_hypre_type'
        petsc_options_value = 'fgmres    300                1e-2      hypre    right        boomeramg'
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
  nl_rel_tol = 1e-12
[]

[Outputs]
  print_linear_residuals = false
  [exo]
    type = Exodus
    execute_on = 'final'
    hide = 'pavg'
  []
[]
