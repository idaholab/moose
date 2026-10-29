  [left_wall]
    type = SolidWall1Phase
    input = pipe:in
  []

  [pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 1

    A = 1.0e-4
    D_h = dh_fn

    f = 0.0

    fp = eos
  []

  [right_wall]
    type = SolidWall1Phase
    input = pipe:out
  []
