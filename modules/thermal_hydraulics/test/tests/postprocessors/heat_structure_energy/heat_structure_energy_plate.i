# Tests the HeatStructureEnergy post-processor for a plate geometry.
#
# The dimensions of the heat structure are:
#   x in (x1, x2) = (0, 2) => length (x-direction) = 2
#   y region 1: y in (y1, y2) = (0, 4)
#   y region 2: y in (y2, y3) = (4, 7)
#     => widths (y-direction) = [4, 3]
#   z in (z1, z2) = (0, 4) => depth (z-direction) = 4
#
# The temperature distribution is the following linear function:
#   T(x,y) = A * x + B * y + C
# where A = 0.2, B = 0.4, C = 0.5.
# The integral of this function w.r.t. y = (y2, y3) is
#   A * x * dy2 + 0.5 * B * (y3^2 - y2^2) + C * dy2
# where dy2 = y3 - y2. The integral of this w.r.t. x = (x1, x2) is
#   A * dy2 * 0.5 * (x2^2 - x1^2) + B * dx * 0.5 * (y3^2 - y2^2) + C * dy2 * dx
# where dx = x2 - x1. Substituting values gives int(T) = 17.4
#
# The post-processor computes the integral
#   rho2 * cp2 * int_x int_y2 T(x, y) * P(y) * dy * dx
# Here P(y) is equal to the depth: P(y) = depth = 4
#
# The relevant heat structure material properties are
#   rho2 = 3
#   cp2 = 5
#
# Finally, rho2 * cp2 * int(T) * P = 1044.
#
# For a test variation using a reference temperature of T_ref = 0.5,
# rho2 * cp2 * int(T - T_ref) * P = 864.

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
    expression = '0.2 * x + 0.4 * y + 0.5'
  []
[]

[Components]
  [heat_structure]
    type = HeatStructurePlate

    position = '0 0 0'
    orientation = '1 0 0'
    length = 2.0
    depth = 4.0
    n_elems = 5

    names = 'region1 region2'
    materials = 'region1-mat region2-mat'
    widths = '4.0 3.0'
    n_part_elems = '5 5'

    initial_T = T0_fn
  []
[]

[Postprocessors]
  [E_tot]
    type = ADHeatStructureEnergy
    block = 'heat_structure:region2'
    plate_depth = 4.0
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
  file_base = 'heat_structure_energy_plate'
  csv = true
  execute_on = 'initial'
[]
