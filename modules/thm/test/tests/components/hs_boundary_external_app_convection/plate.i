# This input file tests HSBoundaryExternalAppConvection by checking energy
# conservation.
#
# The gold value should be the following:
#   E_change = scale * heat_flux * A * t
# where
#   heat_flux = htc * (T_ambient - T_hs)
#   A = L * depth

T_hs = 300
T_ambient = 500
htc = 100
t = 0.001

L = 2
thickness = 0.5
depth = 0.6

# SS 316
density = 8.0272e3
specific_heat_capacity = 502.1
conductivity = 16.26

scale = 0.8

[AuxVariables]
  [./T_ext]
    initial_condition = ${T_ambient}
  [../]
  [./htc_ext]
    initial_condition = ${htc}
  [../]
[]

[HeatStructureMaterials]
  [./hs_mat]
    type = SolidMaterialProperties
    rho = ${density}
    Cp = ${specific_heat_capacity}
    k = ${conductivity}
  [../]
[]

[Components]
  [./hs]
    type = HeatStructurePlate
    orientation = '0 0 1'
    position = '0 0 0'
    length = ${L}
    n_elems = 10

    depth = ${depth}
    widths = '${thickness}'
    n_part_elems = '10'
    materials = 'hs_mat'
    names = 'region'

    initial_T = ${T_hs}
  [../]

  [./ambient_convection]
    type = HSBoundaryExternalAppConvection
    boundary = 'hs:outer'
    hs = hs
    T_ext = T_ext
    htc_ext = htc_ext
    scale_pp = bc_scale_pp
  [../]
[]

[Postprocessors]
  [./bc_scale_pp]
    type = FunctionValuePostprocessor
    function = ${scale}
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./E_hs]
    type = HeatStructureEnergy
    block = 'hs:region'
    plate_depth = ${depth}
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./E_hs_change]
    type = ChangeOverTimePostprocessor
    postprocessor = E_hs
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
[]

[Executioner]
  type = Transient

  [./TimeIntegrator]
    type = ActuallyExplicitEuler
    solve_type = lumped
  [../]
  dt = ${t}
  num_steps = 1
  abort_on_solve_fail = true
[]

[Outputs]
  [./out]
    type = CSV
    show = 'E_hs_change'
    execute_on = 'FINAL'
  [../]
[]
