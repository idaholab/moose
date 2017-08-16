[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
[]

[GlobalParams]
  op_num = 2
  var_name_base = gr
[]

[Variables]
  [./gr0]
    [./InitialCondition]
      type = FunctionIC
      function = 'd:=(x-y)*80;if(d<pi&d>-pi,sin(d/2)/2+0.5,if(d<0,0,1))'
    [../]
  [../]
  [./gr1]
    [./InitialCondition]
      type = FunctionIC
      function = 'd:=(x-y)*80;1-if(d<pi&d>-pi,sin(d/2)/2+0.5,if(d<0,0,1))'
    [../]
  [../]
[]

[Postprocessors]
  [./area]
    type = GrainBoundaryArea
    grains_per_side = 2
  [../]
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]
