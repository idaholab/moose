T_initial = 500
T_ref = ${T_initial}

T_ambient = 300
htc_ambient = 100

L_uncooled = 1.0
L_cooled = 1.0
diameter = 0.01

# Suppose that there are 10 rectangular, 1-mm-thick fins of height 1 cm over the length
# of the cooled section.
n_fin = 10
h_fin = 0.01
t_fin = 0.001
A_fin_single = ${fparse (2 * h_fin + t_fin ) * L_cooled}
A_fin = ${fparse n_fin * A_fin_single}
A_cooled = ${fparse pi * diameter * L_cooled}
A_total = ${fparse A_fin + A_cooled - n_fin * t_fin * L_cooled}
fin_area_fraction = ${fparse A_fin / A_total}
area_increase_factor = ${fparse A_total / A_cooled}
fin_perimeter_area_ratio = ${fparse (2 * L_cooled + 2 * t_fin) / (L_cooled * t_fin)}

k_fin = 15.0

n_elems_uncooled = 10
n_elems_cooled = 10
n_elems_radial = 5

[SolidProperties]
  [sp_ss316]
    type = ThermalSS316Properties
  []
[]

[FunctorMaterials]
  [fin_efficiency_fmat]
    type = FinEfficiencyFunctorMaterial
    fin_height = ${h_fin}
    fin_perimeter_area_ratio = ${fparse fin_perimeter_area_ratio}
    heat_transfer_coefficient = ${htc_ambient}
    thermal_conductivity = ${k_fin}
    fin_efficiency_name = fin_efficiency
  []
  [fin_enhancement_fmat]
    type = FinEnhancementFactorFunctorMaterial
    fin_efficiency = fin_efficiency
    fin_area_fraction = ${fin_area_fraction}
    area_increase_factor = ${area_increase_factor}
    fin_enhancement_factor_name = fin_enhancement
  []
[]

[Components]
  [pipe_with_fins]
    type = HeatStructureCylindrical
    orientation = '0 0 1'
    position = '0 0 0'
    length = '${L_uncooled} ${L_cooled}'
    n_elems = '${n_elems_uncooled} ${n_elems_cooled}'
    axial_region_names = 'uncooled cooled'

    names = 'body'
    widths = '${diameter}'
    n_part_elems = '${n_elems_radial}'
    solid_properties = 'sp_ss316'
    solid_properties_T_ref = '${T_ref}'

    initial_T = ${T_initial}
  []
  [pipe_without_fins]
    type = HeatStructureCylindrical
    orientation = '0 0 1'
    position = '0 0.02 0'
    length = '${L_uncooled} ${L_cooled}'
    n_elems = '${n_elems_uncooled} ${n_elems_cooled}'
    axial_region_names = 'uncooled cooled'

    names = 'body'
    widths = '${diameter}'
    n_part_elems = '${n_elems_radial}'
    solid_properties = 'sp_ss316'
    solid_properties_T_ref = '${T_ref}'

    initial_T = ${T_initial}
  []

  [pipe_with_fins_convection]
    type = HSBoundaryAmbientConvection
    boundary = 'pipe_with_fins:cooled:outer'
    hs = pipe_with_fins
    T_ambient = ${T_ambient}
    htc_ambient = ${htc_ambient}
    scale = fin_enhancement
  []
  [pipe_without_fins_convection]
    type = HSBoundaryAmbientConvection
    boundary = 'pipe_without_fins:cooled:outer'
    hs = pipe_without_fins
    T_ambient = ${T_ambient}
    htc_ambient = ${htc_ambient}
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  dt = 10
  num_steps = 5

  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 10
[]

[Outputs]
  exodus = true
[]
