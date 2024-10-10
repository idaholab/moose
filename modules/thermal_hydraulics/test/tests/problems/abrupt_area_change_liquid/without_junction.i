# Version without junction

!include base_params.i
!include base.i

[Functions]
  [A_fn]
    type = PiecewiseConstant
    axis = x
    x = '0 ${xR}'
    y = '${AL} ${AR}'
  []
[]

[Components]
  [pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = ${fparse lengthL + lengthR}
    n_elems = ${fparse NL + NR}
    A = A_fn
  []

  [left_boundary]
    type = FreeBoundary1Phase
    input = 'pipe:in'
  []

  [right_boundary]
    type = FreeBoundary1Phase
    input = 'pipe:out'
  []
[]

[VectorPostprocessors]
  [vpp]
    type = ADSampler1DReal
    block = 'pipe'
    property = 'p vel'
    sort_by = x
    execute_on = 'FINAL'
  []
[]

[Outputs]
  file_base = 'without_junction'
[]
