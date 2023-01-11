# Tests the HeatStructureEnergyRZ post-processor for a cylinder geometry.
#
# The heat structure will consist of 5 units of the following geometry:
#   x in (x1, x2) = (0, 2) => length (x-direction) = 2
#   inner radius = 2
#   region widths: [4, 3]
#     => y region 1: y in (y1, y2) = (2, 6)
#     => y region 2: y in (y2, y3) = (6, 9)
#
# The temperature distribution is the following linear function:
#   T(x,y) = A * x + B * y + C
# where A = 0.2, B = 0.4, C = 0.5.
# The integral of T(x,y) * y w.r.t. y = (y2, y3) is
#   1.0/3.0 * B * (y3^3 - y2^3) + 0.5 * (A * x + C) * (y3^2 - y2^2)
# The integral of this w.r.t. x = (x1, x2) is
#   1.0/3.0 * B * (y3^3 - y2^3) * dx + 0.5 * (0.5 * A * (x2^2 - x1^2) + C * dx) * (y3^2 - y2^2)
# where dx = x2 - x1.
#
# The post-processor computes the integral
#   n_units * 2 pi * rho2 * cp2 * int_x int_y2 T(x, y) * y * dy * dx,
# where n_units = 5.
#
# The relevant heat structure material properties are
#   rho2 = 3
#   cp2 = 5
#
# Finally, n_units * 2 pi * rho2 * cp2 * int(T * y) = 7.930950653987433e+04

[HeatStructureMaterials]
  [region1-mat]
    type = SolidMaterialProperties
    k = 1
    cp = 1
    rho = 1
  []
  [region2-mat]
    type = SolidMaterialProperties
    k = 1
    cp = 5
    rho = 3
  []
[]

[Functions]
  [T0_fn]
    type = ParsedFunction
    expression = '0.2 * x + 0.4 * (y - 2) + 0.5'
  []
[]

[Components]
  [heat_structure]
    type = HeatStructureCylindrical
    num_rods = 5

    position = '0 2 0'
    orientation = '1 0 0'
    inner_radius = 2.0
    length = 2.0
    n_elems = 50

    names = 'region1 region2'
    materials = 'region1-mat region2-mat'
    widths = '4.0 3.0'
    n_part_elems = '5 50'

    initial_T = T0_fn
  []
[]

[Postprocessors]
  [E_tot]
    type = ADHeatStructureEnergyRZ
    block = 'heat_structure:region2'
    n_units = 5
    axis_point = '0 2 0'
    axis_dir = '1 0 0'
    execute_on = 'initial'
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
  file_base = 'heat_structure_energy_cylinder'
  [csv]
    type = CSV
    precision = 15
    execute_on = 'initial'
  []
[]
