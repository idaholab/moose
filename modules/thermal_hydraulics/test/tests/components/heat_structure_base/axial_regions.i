# This input file is used to test the ability to specify axial regions of a heat
# structure. A heat structure is split into 3 axial regions, and a boundary
# condition is applied to only the bottom 2 regions. A single time step is
# taken, and the output should show heat transfer only at the bottom 2
# boundaries.

[HeatStructureMaterials]
  [hs_mat]
    type = SolidMaterialProperties
    k = 5
    cp = 300
    rho = 100
  []
[]

[Components]
  [hs]
    type = HeatStructurePlate
    position = '1 2 3'
    orientation = '0 0 1'
    depth = 1

    length = '3 2 1'
    n_elems = '2 4 3'

    names = 'radialregion'
    widths = '0.5'
    n_part_elems = '3'
    materials = 'hs_mat'

    initial_T = 300
  []

  [hs_boundary]
    type = HSBoundaryHeatFlux
    boundary = 'hs:region1:outer hs:region2:outer'
    hs = hs
    q = 1000
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
  scheme = 'bdf2'

  start_time = 0
  dt = 1
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = PJFNK
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 10

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]


[Outputs]
  exodus = true
[]
