[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 100
    ny = 100
  []
  [manyblocks]
    input = gen
    type = ElemUniqueSubdomainsGenerator
  []
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[Materials]
  [mat_props]
    type = GenericConstantMaterial
    prop_names = diffusivity
    prop_values = 2
  []
[]

[UserObjects]
[]

[Postprocessors]
  [avg_flux_right]
    # Computes -\int(exp(y)+1) from 0 to 1 which is -2.718281828
    type = SideDiffusiveFluxAverage
    variable = u
    boundary = right
    diffusivity = diffusivity
  []
  [u1_avg]
    type = ElementAverageValue
    variable = u
    execute_on = 'initial timestep_end'
  []

  [u2_avg]
    type = ElementAverageValue
    variable = u
    execute_on = 'initial timestep_end'
  []

  [diff]
    type = DifferencePostprocessor
    value1 = u1_avg
    value2 = u2_avg
    execute_on = 'initial timestep_end'
  []
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]
