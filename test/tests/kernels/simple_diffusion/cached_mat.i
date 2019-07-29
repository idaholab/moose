[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 5
[]

[Materials]
  [m1]
    type = IncrementMaterial
    prop_names = 'a'
    prop_values = '0'
  []
  #[parsedmat]
  #  type = ParsedMaterial
  #  material_property_names = 'm1'
  #  f_name = parsedmat
  #  function = 'm1*0 + 1'
  #[]
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [matvals]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [matvals]
    type = MaterialRealAux
    variable = matvals
    property = mat_prop
    #execute_on = 'nonlinear timestep_begin'
  []
[]

[Kernels]
  [diff]
    #type = MatDiffusion
    type = Diffusion
    variable = u
    #D_name = parsedmat
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
  dt = 0.1
  dtmin = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[UserObjects]
  [u1]
    type = ResetMaterialCache
    execute_on = 'timestep_begin nonlinear'
    #execute_on = 'timestep_begin'
  []
[]

[Outputs]
  exodus = true
  execute_on = 'nonlinear linear timestep_end'
[]
