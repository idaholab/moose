[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./pp_aux]
  [../]
[]

[Functions]
  [./t_func]
    type = ParsedFunction
    expression = t
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.01
  [../]
[]

[AuxKernels]
  [./pp_aux]
    type = PostprocessorAux
    variable = pp_aux
    execute_on = timestep_end
    pp = t_pp
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Postprocessors]
  [./t_pp]
    type = FunctionValuePostprocessor
    function = t_func
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  dt = 1
  num_steps = 5
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
