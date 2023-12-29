[Mesh]
  type = GeneratedMesh
  dim = 2
  elem_type = QUAD4
  displacements = 'disp_x disp_y'
  nx = 2
  ny = 2
[]

[Variables]
  [./disp_x]
    block = 0
  [../]
  [./disp_y]
    block = 0
  [../]
[]

[GlobalParams]
  volumetric_locking_correction=true
[]

[AuxVariables]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
  [./e_yy]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
[]

[Functions]
  [./tdisp]
    type = ParsedFunction
    expression = 0.05*t
  [../]
[]

[UserObjects]
  [./prop_read]
    type = PropertyReadFile
    prop_file_name = 'input_file.txt'
    # Enter file data as prop#1, prop#2, .., prop#nprop
    nprop = 4
    read_type = element
  [../]
[]

[AuxKernels]
  [./stress_yy]
    type = RankTwoAux
    variable = stress_yy
    rank_two_tensor = stress
    index_j = 1
    index_i = 1
    execute_on = timestep_end
    block = 0
  [../]
  [./e_yy]
    type = RankTwoAux
    variable = e_yy
    rank_two_tensor = elastic_strain
    index_j = 1
    index_i = 1
    execute_on = timestep_end
    block = 0
  [../]
[]

[BCs]
  [./fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0
  [../]
  [./fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  [../]
  [./tdisp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = tdisp
  [../]
[]

[Materials]
  [./elasticity_tensor_with_Euler]
    type = ComputeElasticityTensorCP
    block = 0
    C_ijkl = '1.684e5 0.176e5 0.176e5 1.684e5 0.176e5 1.684e5 0.754e5 0.754e5 0.754e5'
    fill_method = symmetric9
    read_prop_user_object = prop_read
  [../]
  [./strain]
    type = ComputeFiniteStrain
    block = 0
    displacements = 'disp_x disp_y'
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = 0
  [../]
[]

[Postprocessors]
  [./stress_yy]
    type = ElementAverageValue
    variable = stress_yy
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./e_yy]
    type = ElementAverageValue
    variable = e_yy
    block = 'ANY_BLOCK_ID 0'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.05

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = -pc_hypre_type
  petsc_options_value = boomerang
  nl_abs_tol = 1e-10
  nl_rel_step_tol = 1e-10
  dtmax = 10.0
  nl_rel_tol = 1e-10
  end_time = 1
  dtmin = 0.05
  num_steps = 1
  nl_abs_step_tol = 1e-10

[]

[Outputs]
  file_base = prop_elem_read_out
  exodus = true
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y'
    use_displaced_mesh = true
  [../]
[]
