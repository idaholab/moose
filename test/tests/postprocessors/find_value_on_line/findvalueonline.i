[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmin = 0
  xmax = 10
[]

[Variables]
  [./phi]
    [./InitialCondition]
      type = FunctionIC
      function = if(x<1,1-x,0)
    [../]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = phi
  [../]
  [./dt]
    type = TimeDerivative
    variable = phi
  [../]
[]

[BCs]
  [./influx]
    type = NeumannBC
    boundary = left
    variable = phi
    value = 1
  [../]
  [./fix]
    type = DirichletBC
    boundary = right
    variable = phi
    value = 0
  [../]
[]

[Postprocessors]
  [./pos]
    type = FindValueOnLine
    target = 0.5
    v = phi
    start_point = '0 0 0'
    end_point = '10 0 0'
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 2.5
[]

[Outputs]
  csv = true
[]
