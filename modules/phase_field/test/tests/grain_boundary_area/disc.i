[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
  xmin = -1.5
  xmax = 1.5
  ymin = -1.5
  ymax = 1.5
[]

[GlobalParams]
  op_num = 1
  var_name_base = gr
[]

[Variables]
  [./gr0]
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 0
      y1 = 0
      z1 = 0
      radius = 1.0
      int_width = 0.15
      invalue = 1
      outvalue = 0
    [../]
  [../]
[]

[Postprocessors]
  [./area]
    type = GrainBoundaryArea
    grains_per_side = 1
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
