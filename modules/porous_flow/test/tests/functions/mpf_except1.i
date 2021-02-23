[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
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
  [moving_planar_front]
    type = MovingPlanarFront
    start_posn = '1 1 0'
    end_posn = '1 1 0'
    active_length = 1
    distance = t
  []
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 10
[]

[Outputs]
  file_base = mpf_except1.i
  exodus = true
[]
