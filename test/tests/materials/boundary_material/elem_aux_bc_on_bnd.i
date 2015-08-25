[Mesh]
  type = GeneratedMesh
  dim = 2

  xmin = 0
  xmax = 1
  nx = 3

  ymin = 0
  ymax = 1
  ny = 3
[]

[AuxVariables]
   [./foo]
     order = CONSTANT
     family = MONOMIAL
   [../]
[]

[Variables]
   [./temp]
    initial_condition = 1
  [../]
[]

[AuxKernels]
  [./copy_bar]
    type = MaterialRealAux
    property = bar
    variable = foo
    boundary = right
    execute_on = timestep_end
  [../]
[]

[Kernels]
  [./heat]
    type = CoefDiffusion
    variable = temp
    coef = 1
  [../]
[]

[BCs]
  [./leftt]
    type = DirichletBC
    boundary =  left
    value    =  2
    variable =  temp
  [../]
[]

[Materials]
  [./thermal_cond]
    type = GenericConstantMaterial
    prop_names = 'bar'
    prop_values = '1'
    block = 0
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 1
  end_time = 1
[]

[Outputs]
  exodus = true
[]
