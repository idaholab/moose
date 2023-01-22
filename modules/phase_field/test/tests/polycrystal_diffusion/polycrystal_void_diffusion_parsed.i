[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
  xmax = 200
  ymax = 200
[]

[GlobalParams]
  op_num = 4
  grain_num = 4
  var_name_base = gr
  int_width = 8
  radius = 20
  bubspac = 1
  numbub = 1
[]

[AuxVariables]
  [bnds]
  []
[]

[AuxKernels]
  [bnds]
    type = BndsCalcAux
    variable = bnds
    v = 'gr0 gr1 gr2 gr3'
    execute_on = 'INITIAL'
  []
[]

[Variables]
  [PolycrystalVariables]
  []
  [c]
  []
[]

[ICs]
  [./PolycrystalICs]
    [./PolycrystalVoronoiVoidIC]
      invalue = 1.0
      outvalue = 0.0
      polycrystal_ic_uo = voronoi
      rand_seed = 10
    [../]
  [../]
  [./bubble_IC]
    variable = c
    type = PolycrystalVoronoiVoidIC
    structure_type = voids
    invalue = 1.0
    outvalue = 0.0
    polycrystal_ic_uo = voronoi
    rand_seed = 10
  [../]
[]

[Materials]
  [Diff_v]
    type = PolycrystalDiffusivity
    c = c
    v = 'gr0 gr1 gr2 gr3'
    diffusivity = diffusivity
    outputs = exodus
    output_properties = 'diffusivity'
  []
  [./hb]
    type = DerivativeParsedMaterial
    property_name = hb
    coupled_variables = 'c'
    expression = 'c * c * c * (6 * c * c - 15 * c + 10)'
  [../]
  [./hm]
    type = DerivativeParsedMaterial
    property_name = hm
    coupled_variables = 'c'
    material_property_names = 'hb'
    expression =  '(1-hb)'
  [../]
[]

[UserObjects]
  [voronoi]
    type = PolycrystalVoronoi
    rand_seed = 1268
  []
[]

[Kernels]
  [bubble]
    type = TimeDerivative
    variable = c
  []
  [gr0]
    type = TimeDerivative
    variable = gr0
  []
  [gr1]
    type = TimeDerivative
    variable = gr1
  []
  [gr2]
    type = TimeDerivative
    variable = gr2
  []
  [gr3]
    type = TimeDerivative
    variable = gr3
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'
  solve_type = 'PJFNK'
  l_tol = 1.0e-4
  l_max_its = 30
  nl_max_its = 20
  nl_rel_tol = 1.0e-9
  num_steps = 1
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
  exodus = true
[]
