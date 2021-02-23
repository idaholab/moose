[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 15
  ny = 15
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [dummy]
    type = TimeDerivative
    variable = u
  []
[]

[Functions]
  [dist]
    type = PiecewiseLinear
    x = '1 10'  # time
    y = '0 9'   # distance
  []
  [moving_planar_front]
    type = MovingPlanarFront
    start_posn = '1 1 0'
    end_posn = '2 2 0' # it does not matter that dist exceeds this
    active_length = 5
    activation_time = 1
    deactivation_time = 9
    distance = dist
  []
[]

[AuxVariables]
  [mpf]
  []
[]

[AuxKernels]
  [mpf]
    type = FunctionAux
    variable = mpf
    function = moving_planar_front
  []
[]

[Executioner]
  type = Transient
  dt = 0.5
  end_time = 10
[]

[Outputs]
  file_base = mpf1
  exodus = true
[]
