# Tests the SideFluxIntegralRZ post-processor, both for an axial boundary and
# a radial boundary.
#
# The temperature distribution and thermal conductivity are set as follows:
#   T(x,r) = xr
#   k = 5
#
# First, the following axial boundary is tested:
#   (x,r) in x0 X (r0, r1),
#   x0 = 3, r0 = 1.5, r1 = 2.2
# with n = +e_x (positive x-direction).
# In this case, the integral of [-k grad(T) * n] is
#   Q = -2/3 pi k (r1^3 - r0^3)
#     = -76.16267789852857
#
# Next, the following radial boundary is tested:
#  (x,r) in (x0,x1) X r0
#  x0 = 0, x1 = 5, r0 = 1.5
# with n = -e_r (negative r-direction).
# In this case, the integral of [-k grad(T) * n] is
#   Q = pi * r0 * k (x1^2 - x0^2)
#     = 589.0486225480862

R_i = 1.0

[Functions]
  [T_fn]
    type = ParsedFunction
    expression = 'x * y'
  []
[]

[HeatStructureMaterials]
  [hsmat]
    type = SolidMaterialProperties
    k = 5
    cp = 1
    rho = 1
  []
[]

[Components]
  [heat_structure]
    type = HeatStructureCylindrical

    position = '0 0 0'
    orientation = '1 0 0'
    length = '3 2'
    n_elems = '5 4'
    axial_region_names = 'axial1 axial2'

    inner_radius = ${R_i}
    names = 'radial1 radial2'
    materials = 'hsmat hsmat'
    widths = '0.5 0.7'
    n_part_elems = '2 3'

    initial_T = T_fn
  []
[]

[Postprocessors]
  [Q_axial]
    type = ADSideFluxIntegralRZ
    boundary = heat_structure:radial2:axial1:axial2
    variable = T_solid
    diffusivity = thermal_conductivity
    axis_point = '0 0 0'
    axis_dir = '1 0 0'
    execute_on = 'INITIAL'
  []
  [Q_radial]
    type = ADSideFluxIntegralRZ
    boundary = heat_structure:radial1:radial2
    variable = T_solid
    diffusivity = thermal_conductivity
    axis_point = '0 0 0'
    axis_dir = '1 0 0'
    execute_on = 'INITIAL'
  []
[]

[Problem]
  solve = false
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
