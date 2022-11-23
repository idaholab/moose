# Tests DiscreteLineSegmentInterface

[AuxVariables]
  [ax_coord]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxKernels]
  [ax_coord_aux]
    type = DiscreteLineSegmentInterfaceTestAux
    variable = ax_coord
    position = '5 -4 2'
    orientation = '-1 3 -5'
    rotation = 60
    length = 10.0
    n_elems = 20
    execute_on = 'INITIAL'
  []
[]

[HeatStructureMaterials]
  [hs_mat]
    type = SolidMaterialProperties
    k = 1
    cp = 1
    rho = 1
  []
[]

[Components]
  [hs]
    type = HeatStructureCylindrical
    position = '5 -4 2'
    orientation = '-1 3 -5'
    rotation = 60
    length = 10.0
    n_elems = 20

    names = 'region1 region2 region3'
    widths = '1.0 3.0 2.0'
    n_part_elems = '2 6 8'
    materials = 'hs_mat hs_mat hs_mat'

    initial_T = 300
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  exodus = true
  hide = 'T_solid'
[]
