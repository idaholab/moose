[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 1
  xmax = 2
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./stretch]
  [../]
[]

[Functions]
  [./stretch_func]
    type = ParsedFunction
    expression = t
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
    use_displaced_mesh = true
  [../]
[]

[AuxKernels]
  [./interpolation]
    type = CoupledDirectionalMeshHeightInterpolation
    variable = disp_x
    direction = x
    execute_on = timestep_begin
    coupled_var = stretch
  [../]
  [./stretch_aux]
    type = FunctionAux
    variable = stretch
    function = stretch_func
    execute_on = timestep_begin
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
    use_displaced_mesh = true
  [../]
  [./right]
    type = NeumannBC
    variable = u
    boundary = right
    value = 1
    use_displaced_mesh = true
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 1

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'



  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
