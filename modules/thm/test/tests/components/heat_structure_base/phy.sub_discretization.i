#
# Testing the ability to discretize the HeatStructure by dividing it into
# axial subsections
#

[GlobalParams]
[]

[HeatStructureMaterials]
  [fuel-mat]
    type = SolidMaterialProperties
    k = 3.65
    cp = 288.734
    rho = 1.0412e2
  []
  [gap-mat]
    type = SolidMaterialProperties
    k = 1.084498
    cp = 1.0
    rho = 1.0
  []
  [clad-mat]
    type = SolidMaterialProperties
    k = 16.48672
    cp = 321.384
    rho = 6.6e1
  []
[]

[Components]
  [hs]
    type = HeatStructureCylindrical
    position = '0 0 1'
    orientation = '1 0 0'

    axial_region_names = 'reg1 reg2'
    length = '2.0 1.6576'
    n_elems = '7   4'

    names = 'FUEL GAP CLAD'
    widths = '0.0046955  0.0000955  0.000673'
    n_part_elems = '10 3 3'
    materials = 'fuel-mat gap-mat clad-mat'

    initial_T = 300
  []

  [temp_outside]
    type = HSBoundarySpecifiedTemperature
    hs = hs
    boundary = hs:outer
    T = 300
  []
[]

[Preconditioning]
  [SMP_PJFNK]
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

  solve_type = 'PJFNK'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  nl_max_its = 30

  l_tol = 1e-4
  l_max_its = 300
[]


[Outputs]
  [out]
    type = Exodus
  []
  [console]
    type = Console
    execute_scalars_on = none
  []
[]
