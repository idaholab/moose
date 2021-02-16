# Used for testing that the inner radial boundaries of a heat structure are
# created correctly. A SideValueSampler VPP samples a variable along an inner
# radial boundary and the test verifies that the correct space points and
# variable values are recovered.

[Functions]
  [initial_T_fn_ax_x]
    type = PiecewiseLinear
    axis = x
    x = '0 5 10'
    y = '300 500 1000'
  []
  [initial_T_fn_ax_y]
    type = PiecewiseLinear
    axis = y
    x = '0 0.75 1.0 4.0 6.0'
    y = '0 0    1.0 1.5 2.0'
  []
  [initial_T_fn]
    type = CompositeFunction
    functions = 'initial_T_fn_ax_x initial_T_fn_ax_y'
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

    position = '0 0 0'
    orientation = '1 0 0'
    length = 10.0
    n_elems = 20

    names = 'region1 region2 region3'
    widths = '1.0 3.0 2.0'
    n_part_elems = '2 6 8'
    materials = 'hs_mat hs_mat hs_mat'

    initial_T = initial_T_fn
  []
[]

[VectorPostprocessors]
  [test_vpp]
    type = SideValueSampler
    variable = T_solid
    boundary = 'hs:region1:region2'
    sort_by = x
    execute_on = 'INITIAL'
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
  csv = true
  execute_on = 'INITIAL'
[]
