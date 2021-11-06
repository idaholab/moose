#
# Single block thermal input with a line value sampler
# https://mooseframework.inl.gov/modules/heat_conduction/tutorials/introduction/therm_step02.html
#

[Mesh]
  [generated]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmax = 2
    ymax = 1
  []
[]

[Variables]
  [T]
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = T
  []
[]

[Materials]
  [thermal]
    type = HeatConductionMaterial
    thermal_conductivity = 45.0
  []
[]

[BCs]
  [t_left]
    type = DirichletBC
    variable = T
    value = 300
    boundary = 'left'
  []
  [t_right]
    type = FunctionDirichletBC
    variable = T
    function = '300+5*t'
    boundary = 'right'
  []
[]

[Executioner]
  type = Transient
  end_time = 5
  dt = 1
[]

[VectorPostprocessors]
  [t_sampler]
    type = LineValueSampler
    variable = T
    start_point = '0 0.5 0'
    end_point = '2 0.5 0'
    num_points = 20
    sort_by = x
  []
[]

[Outputs]
  exodus = true
  [csv]
    type = CSV
    file_base = therm_step02a_out
    execute_on = final
  []
[]
