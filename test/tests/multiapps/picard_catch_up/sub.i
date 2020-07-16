[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Variables]
  [./v]
  [../]
[]

[AuxVariables]
  [./u]
  [../]
[]

[Kernels]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]
  [./force_v]
    type = CoupledForce
    variable = v
    v = u
  [../]
  [./nan]
    type = NanAtCountKernel
    variable = v
    count = 32
  [../]
[]

[BCs]
  [./left_v]
    type = DirichletBC
    variable = v
    preset = false
    boundary = left
    value = 1
  [../]
  [./right_v]
    type = FunctionDirichletBC
    variable = v
    preset = false
    boundary = right
    function = 't + 1'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-10
  snesmf_reuse_base = false
[]

[Outputs]
  exodus = true
[]
