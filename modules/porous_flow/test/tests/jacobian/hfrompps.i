[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [pressure]
  []

  [temperature]
  []
[]

[ICs]
  [pressure_ic]
    type = ConstantIC
    variable = pressure
    value = 1
  []
  [temperature_ic]
    type = ConstantIC
    variable = temperature
    value = 4
  []
[]

[Kernels]
  [p_td]
    type = TimeDerivative
    variable = pressure
  []

  [energy_dot]
    type = TimeDerivative
    variable = temperature
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 1000
    thermal_expansion = 0
  []
[]

[DiracKernels]
  [source_h]
    type = PorousFlowPointEnthalpySourceFromPostprocessor
    variable = temperature
    mass_flux = mass_flux_in
    point = '0.5 0.5 0'
    T_in = T_in
    pressure = pressure
    fp = simple_fluid
  []
[]

[Preconditioning]
  [preferred]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -snes_test_err'
    petsc_options_value = ' lu      1e-6'
  []
[]

[Postprocessors]
  [mass_flux_in]
    type = FunctionValuePostprocessor
    function = 1
    execute_on = 'initial timestep_end'
  []

  [T_in]
    type = FunctionValuePostprocessor
    function = 1
    execute_on = 'initial timestep_end'
  []
[]


[Executioner]
  type = Transient
  solve_type = Newton
  nl_abs_tol = 1e-14
  dt = 1
  num_steps = 1
[]
