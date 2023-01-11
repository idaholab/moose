
[Mesh]
  type = GeneratedMesh
  nx = 10
  ny = 10
  xmax = 1
  ymax = 1
  dim = 2
[]

[Variables]
  [./u]
  [../]
  [./v]
  [../]
[]

[Kernels]
  [./conv]
    type = ConservativeAdvection
    variable = u
    velocity = '0 1 0'
  [../]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./src]
    type = BodyForce
    variable = u
    function = ffn
  [../]
  [./diffv]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 2
  [../]
  [./right]
    type = ChannelGradientBC
    variable = u
    boundary = right
    channel_gradient_pps = channel_gradient
    axis = y
    h_name = h
  [../]
  [./top]
    type = OutflowBC
    variable = u
    boundary = top
    velocity = '0 1 0'
  [../]
  [./leftv]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  [../]
  [./rightv]
    type = DirichletBC
    variable = v
    boundary = right
    value = 1
  [../]
[]

[Materials]
  [./mat]
    type = GenericConstantMaterial
    prop_names = 'h'

    #Nu = 4
    #k = 1
    #half_channel_length = 0.5
    #h=Nu*k/half_channel_length
    prop_values = '8'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]

[VectorPostprocessors]
  [./lv1]
    num_points = 30
    start_point = '0 0 0'
    end_point = '0 1 0'
    sort_by = 'y'
    variable = u
    type = LineValueSampler
    execute_on = 'timestep_begin nonlinear timestep_end linear'
  [../]
  [./lv2]
    num_points = 30
    start_point = '1 0 0'
    end_point =   '1 1 0'
    sort_by = 'y'
    variable = v
    type = LineValueSampler
    execute_on = 'timestep_begin nonlinear timestep_end linear'
  [../]
  [./channel_gradient]
    lv1 = lv1
    lv2 = lv2
    var1 = u
    var2 = v
    axis = y
    type = ChannelGradientVectorPostprocessor
    execute_on = 'timestep_begin nonlinear timestep_end linear'
  [../]
[]

[Functions]
  [./ffn]
    type = ParsedFunction
    expression = '1'
  [../]
[]
