# This input file tests checks that T_ext and htc_ext are properly
# transferred from the master app.

T_hs = 300

L = 1
thickness = 0.5
depth = 0.6

density = 8.0272e3
specific_heat_capacity = 502.1
conductivity = 20

scale = 0.8


[HeatStructureMaterials]
  [hs_mat]
    type = SolidMaterialProperties
    rho = ${density}
    cp = ${specific_heat_capacity}
    k = ${conductivity}
  []
[]

[Components]
  [hs]
    type = HeatStructurePlate
    orientation = '1 0 0'
    position = '0 0 0'
    length = ${L}
    n_elems = 10

    depth = ${depth}
    widths = '${thickness}'
    n_part_elems = '10'
    materials = 'hs_mat'
    names = 'region'

    initial_T = ${T_hs}
  []

  [ambient_convection]
    type = HSBoundaryExternalAppConvection
    boundary = 'hs:outer'
    hs = hs
    scale = ${scale}
  []
[]

[Executioner]
  type = Transient
  abort_on_solve_fail = true
[]

[Outputs]
  exodus = true
[]
