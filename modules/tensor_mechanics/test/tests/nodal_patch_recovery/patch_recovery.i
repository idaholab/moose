[GlobalParams]
  displacements = 'ux uy'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[UserObjects]
  [stress_xx_patch]
    type = NodalPatchRecoveryMaterialProperty
    patch_polynomial_order = FIRST
    property = 'stress'
    component = '0 0'
    execute_on = 'TIMESTEP_END'
  []
  [stress_yy_patch]
    type = NodalPatchRecoveryMaterialProperty
    patch_polynomial_order = FIRST
    property = 'stress'
    component = '1 1'
    execute_on = 'TIMESTEP_END'
  []
[]

[AuxVariables]
  [stress_xx_recovered]
    order = FIRST
    family = LAGRANGE
  []
  [stress_yy_recovered]
    order = FIRST
    family = LAGRANGE
  []
[]

[Functions]
  [tdisp]
    type = ParsedFunction
    expression = 0.01*t
  []
[]

[Modules/TensorMechanics/Master/all]
  strain = FINITE
  add_variables = true
[]

[AuxKernels]
  [stress_xx_recovered]
    type = NodalPatchRecoveryAux
    variable = stress_xx_recovered
    nodal_patch_recovery_uo = stress_xx_patch
    execute_on = 'TIMESTEP_END'
  []
  [stress_yy_recovered]
    type = NodalPatchRecoveryAux
    variable = stress_yy_recovered
    nodal_patch_recovery_uo = stress_yy_patch
    execute_on = 'TIMESTEP_END'
  []
[]

[BCs]
  [fix_y]
    type = DirichletBC
    variable = uy
    boundary = 'bottom'
    value = 0
  []
  [fix_x]
    type = DirichletBC
    variable = ux
    boundary = 'top bottom'
    value = 0
  []
  [disp_y]
    type = FunctionDirichletBC
    variable = uy
    boundary = 'top'
    function = tdisp
    preset = false
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensorCP
    C_ijkl = '1.684e5 1.214e5 1.214e5 1.684e5 1.214e5 1.684e5 0.754e5 0.754e5 0.754e5'
    fill_method = symmetric9
  []
  [stress]
    type = ComputeMultipleCrystalPlasticityStress
    crystal_plasticity_models = 'trial_xtalpl'
    tan_mod_type = exact
  []
  [trial_xtalpl]
    type = CrystalPlasticityKalidindiUpdate
    number_slip_systems = 12
    slip_sys_file_name = input_slip_sys.txt
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  automatic_scaling = true

  dt = 0.05
  num_steps = 2

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
