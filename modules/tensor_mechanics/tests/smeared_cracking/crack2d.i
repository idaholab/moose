[Mesh]
  type = FileMesh
  file = crack_mesh.e
  uniform_refine = 0
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
    block = 1
  [../]
  [./disp_y]
    block = 1
  [../]
[]

[AuxVariables]
  [./resid_x]
    block = 1
  [../]
  [./resid_y]
    block = 1
  [../]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
    block = 1
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
    block = 1
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
    block = 1
  [../]
#   [./crack_flag_xx]
#     order = CONSTANT
#     family = MONOMIAL
#     block = 1
#   [../]
#   [./crack_flag_yy]
#     order = CONSTANT
#     family = MONOMIAL
#     block = 1
#   [../]
#   [./crack_flag_zz]
#     order = CONSTANT
#     family = MONOMIAL
#     block = 1
#   [../]

[]


[Functions]
  [./tfunc]
    type = ParsedFunction
    value = t
  [../]
[]

[Kernels]

  [./TensorMechanics]
    displacements = 'disp_x disp_y'
    use_displaced_mesh = false
    save_in_disp_x = resid_x
    save_in_disp_y = resid_y
  [../]

[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    variable = stress_xx
    rank_two_tensor = stress
    index_j = 0
    index_i = 0
    execute_on = timestep_end
    block = 1
  [../]
  [./stress_yy]
    type = RankTwoAux
    variable = stress_yy
    rank_two_tensor = stress
    index_j = 1
    index_i = 1
    execute_on = timestep_end
    block = 1
  [../]
  [./stress_zz]
    type = RankTwoAux
    variable = stress_zz
    rank_two_tensor = stress
    index_j = 2
    index_i = 2
    execute_on = timestep_end
    block = 1
  [../]
#   [./crack_flag_xx]
#     type = MaterialVectorAux
#     variable = crack_flag_xx
#     vector = crack_flags
#     index = 0
#     execute_on = timestep_end
#     block = 1
#   [../]
#   [./crack_flag_yy]
#     type = MaterialVectorAux
#     variable = crack_flag_yy
#     vector = crack_flags
#     index = 1
#     execute_on = timestep_end
#     block = 1
#   [../]
#   [./crack_flag_zz]
#     type = MaterialVectorAux
#     variable = crack_flag_zz
#     vector = crack_flags
#     index = 2
#     execute_on = timestep_end
#     block = 1
#   [../]
[]

[BCs]
  [./ydisp]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 2
    function = tfunc
  [../]
  [./yfix]
    type = PresetBC
    variable = disp_y
    boundary = 1
    value = 0
  [../]
  [./xfix]
    type = PresetBC
    variable = disp_x
    boundary = '1 2'
    value = 0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 1
    C_ijkl = '120.0 80.0'
    fill_method = symmetric_isotropic
  [../]

  [./strain]
    type = ComputeSmallStrain
    block = 1
    displacements = 'disp_x disp_y'
  [../]

  [./elastic_stress]
    type = ComputeElasticSmearedCrackingStress
    block = 1
    cracking_release = exponential
    cracking_stress = 1.0
    cracking_residual_stress = 0.1
  [../]

[]

[Postprocessors]
  [./resid_x]
    type = NodalSum
    variable = resid_x
    boundary = 2
  [../]
  [./resid_y]
    type = NodalSum
    variable = resid_y
    boundary = 2
  [../]
[]

# [Preconditioning]
#   active = 'smp'
#   [./smp]
#     type = SMP
#     full = true
#   [../]
# []

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  line_search = 'none'

  l_max_its = 50
  l_tol = 1.0e-5

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  nl_max_its = 15

  dtmax = 100.0
  dtmin = 1.0e-12
  end_time = 3.0e-3
  dt =1.0e-5
  num_steps = 5


   [./TimeStepper]
#     type = SolutionTimeAdaptiveDT
      type = ConstantDT
     dt = 1.0e-5
   [../]

[]

[Outputs]
  exodus = true
  csv = true
  gnuplot = true
[]
