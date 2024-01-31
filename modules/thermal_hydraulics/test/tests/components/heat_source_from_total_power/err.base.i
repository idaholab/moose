[SolidProperties]
  [fuel-mat]
    type = ThermalFunctionSolidProperties
    k = 2.5
    cp = 300.
    rho = 1.032e4
  []
[]

[Components]
  [reactor]
    type = TotalPower
    power = 10
  []

  [hs]
    type = HeatStructureCylindrical
    position = '0 -0.024748 0'
    orientation = '0 0 1'
    length = 3.865
    n_elems = 1

    names = 'fuel'
    widths = '0.004096'
    n_part_elems = '1'
    solid_properties = 'fuel-mat'
    solid_properties_T_ref = '300'

    initial_T = 559.15
  []

  [hgen]
    type = HeatSourceFromTotalPower
    power_fraction = 1
  []
[]

[Executioner]
  type = Transient
  dt = 1.e-2
[]
