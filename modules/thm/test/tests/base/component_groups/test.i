[GlobalParams]
  closures = simple_closures

  initial_p = 1e6
  initial_T = 300
  initial_vel = 0
[]

[FluidProperties]
  [fp_liquid]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1.e9
    cv = 1816
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[HeatStructureMaterials]
  [hx:wall]
    type = SolidMaterialProperties
    k = 1
    cp = 1
    rho = 1
  []
[]

[Components]
  [pri_inlet]
    type = SolidWall1Phase
    input = 'hx/primary:out'
  []

  [pri_outlet]
    type = SolidWall1Phase
    input = 'hx/primary:in'
  []

  # heat exchanger
  [hx]
    n_elems = 2
    length = 1

    [primary]
      type = FlowChannel1Phase
      position = '0 1 0'
      orientation = '1 0 0'
      n_elems = ${n_elems}
      length = ${length}
      A = 1
      f = 1
      fp = fp_liquid
    []

    [wall]
      type = HeatStructurePlate
      position = '0 0 0'
      orientation = '1 0 0'
      materials = hx:wall
      n_elems = ${n_elems}
      length = ${length}
      n_part_elems = 1
      names = 0
      widths = 1
      depth = 1
      initial_T = 300
    []

    [ht_primary]
      type = HeatTransferFromHeatStructure1Phase
      hs = hx/wall
      flow_channel = hx/primary
      hs_side = outer
      Hw = 0
    []

    [ht_secondary]
      type = HeatTransferFromHeatStructure1Phase
      hs = hx/wall
      flow_channel = hx/secondary
      hs_side = inner
      Hw = 0
    []

    [secondary]
      type = FlowChannel1Phase
      position = '0 0 0'
      orientation = '1 0 0'
      n_elems = ${n_elems}
      length = ${length}
      A = 1
      f = 1
      fp = fp_liquid
    []
  []

  [sec_inlet]
    type = SolidWall1Phase
    input = 'hx/secondary:out'
  []
  [sec_outlet]
    type = SolidWall1Phase
    input = 'hx/secondary:in'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  [console]
    type = Console
    system_info = ''
    enable = false
  []
[]

[Debug]
  print_component_loops = true
[]
