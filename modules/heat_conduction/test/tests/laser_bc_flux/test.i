[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    nx = 20
    ny = 10
    ymax = 0.5
    dim = 2
  []
[]

[Variables]
  [temperature]
    initial_condition = 1
  []
[]

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temperature
    diffusion_coefficient = 1
  []
[]

[BCs]
  [radiation_flux]
    type = FunctionRadiativeBC
    variable = temperature
    boundary = 'top'
    emissivity_function = '1'
    Tinfinity = 0
    stefan_boltzmann_constant = 1
  []
  [weld_flux]
    type = GaussianEnergyFluxBC
    variable = temperature
    boundary = 'top'
    P0 = 0.06283185307179587
    R = 0.18257418583505539
    x_beam_coord = 0.5
    y_beam_coord = 0.5
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre    boomeramg'
[]

[Postprocessors]
  [average]
    type = ElementAverageValue
    variable = temperature
  []
[]

[Outputs]
  exodus = true
[]
