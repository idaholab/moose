[GlobalParams]
  initial_p = 1e5
  initial_T = 300
  initial_vel = 0

  closures = simple_closures

  f = 0
  fp = fp

  gravity_vector = '0 0 0'
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[HeatStructureMaterials]
  [m]
    type = SolidMaterialProperties
    rho = 1
    cp = 1
    k = 1
  []
[]

[Components]
  [fch1]
    type = FlowChannel1Phase
    position = '-0.1 0 0'
    orientation = '0 0 1'
    length = 1
    A = 1
    n_elems = 10
  []

  [wall1i]
    type = SolidWall1Phase
    input = fch1:in
  []

  [wall1o]
    type = SolidWall1Phase
    input = fch1:out
  []

  [hs1]
    type = HeatStructureCylindrical
    position = '-0.2 0 0'
    orientation = '0 0 1'
    length = 1
    n_elems = 10
    names = '1 2'
    widths = '0.2 0.3'
    materials = 'm m'
    n_part_elems = '1 1'
    rotation = 90
  []

  [fch2]
    type = FlowChannel1Phase
    position = '0.1 0 0'
    orientation = '0 0 1'
    length = '0.6 0.4'
    A = 1
    n_elems = '5 5'
    axial_region_names = 'longer shorter'
  []

  [wall2i]
    type = SolidWall1Phase
    input = fch2:in
  []

  [wall2o]
    type = SolidWall1Phase
    input = fch2:out
  []

  [hs2]
    type = HeatStructureCylindrical
    position = '0.2 0 0'
    orientation = '0 0 1'
    length = '0.6 0.4'
    axial_region_names = 'longer shorter'
    n_elems = '5 5'
    names = '1 2'
    widths = '0.2 0.3'
    materials = 'm m'
    n_part_elems = '1 1'
    rotation = 270
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 1

  automatic_scaling = true
  nl_abs_tol = 1e-7
[]

[Outputs]
  [map]
    type = ParaviewComponentAnnotationMap
  []
[]
