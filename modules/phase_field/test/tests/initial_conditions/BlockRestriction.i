[Mesh]
  [two_blocks]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1.5 2.4'
    dy = '1.3'
    ix = '5 5'
    iy = '5'
    subdomain_id = '0 1'
  []
[]

[Variables]
  [bicrystal0_0]
  []
  [bicrystal0_1]
  []
  [bicrystal1_0]
  []
  [bicrystal1_1]
  []
  [tricrystal_0]
  []
  [tricrystal_1]
  []
  [tricrystal_2]
  []
  [PolycrystalVariables]
    var_name_base = 'coloring_'
    op_num = 8
  []
  [random_0]
  []
  [random_1]
  []
  [voronoi_0]
  []
  [voronoi_1]
  []
  [voronoi_2]
  []
  [voronoi_3]
  []
  [voronoi_4]
  []
  [voronoi_5]
  []
  [voronoi_6]
  []
  [voronoi_7]
  []
[]

[ICs]
  [PolycrystalICs]
    [BicrystalBoundingBoxIC]
      block = '0'
      var_name_base = 'bicrystal0_'
      op_num = 2
      x1 = 0
      x2 = 1
      y1 = 0
      y2 = 1
    []
    [BicrystalCircleGrainIC]
      block = '0'
      var_name_base = 'bicrystal1_'
      op_num = 2
      x = 2.7
      y = 0.6
      radius = 2
    []
    [Tricrystal2CircleGrainsIC]
      block = '0'
      var_name_base = 'tricrystal_'
      op_num = 3
    []
    [PolycrystalColoringIC]
      block = '0'
      polycrystal_ic_uo = hex_ic
      var_name_base = 'coloring_'
      op_num = 8
    []
    [PolycrystalRandomIC]
      block = '0'
      var_name_base = 'random_'
      op_num = 2
      random_type = 'continuous'
    []
    [PolycrystalVoronoiVoidIC]
      polycrystal_ic_uo = voronoi
      block = '0'
      numbub = 3
      bubspac = 0.02
      radius = 0.05
      invalue = 1
      outvalue = 0.1
      var_name_base = 'voronoi_'
      op_num = 8
    []
  []
[]

[UserObjects]
  [voronoi]
    type = PolycrystalVoronoi
    rand_seed = 10
    int_width = 0
    var_name_base = 'voronoi_'
    op_num = 8
    grain_num = 4
  []
  [hex_ic]
    type = PolycrystalHex
    coloring_algorithm = bt
    var_name_base = 'coloring_'
    op_num = 8
    grain_num = 4
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  exodus = true
[]
