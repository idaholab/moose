# Version with junction

!include base_params.i

# For equivalent comparison to the unsplit case, we reduce the length of the
# right pipe by one element
dx = ${fparse lengthL / NL}
NR_minus_junction = ${fparse NR - 1}
lengthR_minus_junction = ${fparse lengthR - dx}
xR_minus_junction = ${fparse xR + dx}

xJ = ${fparse lengthL + 0.5 * dx}
AJ = ${fparse AL + AR}
RJ = ${fparse sqrt(AJ / (4 * pi))} # A = 4 pi R^2
VJ = ${fparse 4/3 * pi * RJ^3}

!include base.i

[Components]
  [pipeL]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = ${lengthL}
    n_elems = ${NL}
    A = ${AL}
  []

  [pipeR]
    type = FlowChannel1Phase
    position = '${xR_minus_junction} 0 0'
    orientation = '1 0 0'
    length = ${lengthR_minus_junction}
    n_elems = ${NR_minus_junction}
    A = ${AR}
  []

  [junction]
    type = VolumeJunction1Phase
    connections = 'pipeL:out pipeR:in'
    position = '${xJ} 0 0'
    volume = ${VJ}
    initial_vel_x = 0
    initial_vel_y = 0
    initial_vel_z = 0
    scaling_factor_rhoEV = 1e-5
    apply_velocity_scaling = true
  []

  [left_boundary]
    type = FreeBoundary1Phase
    input = 'pipeL:in'
  []

  [right_boundary]
    type = FreeBoundary1Phase
    input = 'pipeR:out'
  []
[]

[VectorPostprocessors]
  [vpp]
    type = ADSampler1DReal
    block = 'pipeL pipeR'
    property = 'p vel'
    sort_by = x
    execute_on = 'FINAL'
  []
[]

[Outputs]
  file_base = 'with_junction'
[]
