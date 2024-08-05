U_in = 1
mu = 1
rho = 1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 3.0
    ymin = 0
    ymax = 1.0
    nx = 30
    ny = 10
    elem_type = QUAD9
  []
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

[ICs]
  [vel]
    type = VectorConstantIC
    variable = vel
    x_value = ${U_in}
  []
[]

[Kernels]
  [mass]
    type = INSADConservativeMass
    variable = p
    velocity = vel
  []
  [mass_kernel]
    type = MassMatrix
    variable = p
    matrix_tags = 'mass'
    density = ${fparse -1/mu}
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
  [walls_momentum]
    type = ADVectorFunctionDirichletBC
    variable = vel
    boundary = 'top'
    preset = true
  []
  [symm_momentum]
    type = INSADMomentumZeroViscousStreeBC
    variable = vel
    boundary = 'bottom'
    pressure = p
    integrate_p_by_parts = true
  []
  [inlet_mass]
    type = INSADConservativeMassWeakDiriBC
    variable = p
    velocity = 'inlet'
    boundary = 'left'
  []
  [inlet_momentum_stress]
    type = INSADMomentumZeroViscousStreeBC
    variable = vel
    boundary = 'left'
    pressure = p
    integrate_p_by_parts = true
  []
  [outlet_mass]
    type = INSADConservativeMassImplicitBC
    variable = p
    velocity = vel
    boundary = 'right'
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '${rho}  ${mu}'
  []
  [mat]
    type = INSADMaterial
    velocity = vel
    pressure = p
  []
[]

[Problem]
  type = NavierStokesProblem
  mass_matrix = 'mass'
  extra_tag_matrices = 'mass'
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
        sides = 'top'
        vars = 'vel'
        petsc_options_iname = '-ksp_type'
        petsc_options_value = 'none'
      []
      [others]
        splitting = 'u p'
        splitting_type  = schur
        petsc_options_iname = '-pc_fieldsplit_schur_fact_type  -pc_fieldsplit_schur_precondition -ksp_gmres_restart -ksp_type -ksp_pc_side -ksp_rtol -ksp_view_pmat'
        petsc_options_value = 'full                            self                              300                fgmres    right        1e-4      binary'
        unside_by_var_boundary_name = 'top'
        unside_by_var_var_name = 'vel'
      []
        [u]
          vars = 'vel'
          unside_by_var_boundary_name = 'top'
          unside_by_var_var_name = 'vel'
          petsc_options_iname = '-pc_type -ksp_type -ksp_rtol -ksp_gmres_restart -ksp_pc_side -pc_factor_mat_solver_type'
          petsc_options_value = 'ilu      gmres     1e-2      300                right        strumpack'
        []
        [p]
          vars = 'p'
          petsc_options = '-ksp_converged_reason'
          petsc_options_iname = '-ksp_type -ksp_gmres_restart -ksp_rtol -pc_type -ksp_pc_side'
          petsc_options_value = 'gmres     300                1e-2      ilu      right'
        []
  []
[]

[Executioner]
  type = Steady
  line_search = none
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]

[Functions]
  [inlet]
    type = ParsedVectorFunction
    expression_x = ${U_in}
  []
  [walls]
    type = ParsedVectorFunction
  []
[]

[Postprocessors]
  [symm]
    type = IsMatrixSymmetric
    binary_mat = binaryoutput
    execute_on = 'timestep_end'
  []
[]
