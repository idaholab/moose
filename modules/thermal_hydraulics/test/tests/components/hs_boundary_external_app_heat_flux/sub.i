# Sub input file.

L = 5.0
radius = 0.01

n_elems_axial = 10
n_elems_radial = 5

T_initial = 300.0
power = 1000.0
t = 10.0
E_change = ${fparse power * t}

rho = 8000.0
cp = 500.0
k = 15.0

[HeatStructureMaterials]
  [hs_mat]
    type = SolidMaterialProperties
    rho = ${rho}
    cp = ${cp}
    k = ${k}
  []
[]

[Components]
  [hs]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '0 0 1'
    length = ${L}
    n_elems = ${n_elems_axial}

    names = 'body'
    widths = '${radius}'
    n_part_elems = '${n_elems_radial}'
    materials = 'hs_mat'

    initial_T = ${T_initial}
  []
  [hs_boundary]
    type = HSBoundaryExternalAppHeatFlux
    hs = hs
    boundary = 'hs:outer'
    heat_flux_name = q_ext
    heat_flux_is_inward = true
    perimeter_ext = P_ext
  []
[]

[Postprocessors]
  [E_hs]
    type = ADHeatStructureEnergyRZ
    block = 'hs:body'
    axis_dir = '0 0 1'
    axis_point = '0 0 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [E_hs_change]
    type = ChangeOverTimePostprocessor
    postprocessor = E_hs
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [E_change_relerr]
    type = RelativeDifferencePostprocessor
    value1 = E_hs_change
    value2 = ${E_change}
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [integral_relerr]
    type = RelativeDifferencePostprocessor
    value1 = hs_boundary_integral
    value2 = ${power}
    execute_on = 'INITIAL TIMESTEP_END'
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

  scheme = bdf2
  dt = ${t}
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = NEWTON
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-8
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 10

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]

[Outputs]
  csv = true
  show = 'E_change_relerr integral_relerr'
  execute_on = 'FINAL'
[]
