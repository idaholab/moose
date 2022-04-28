#
# This test checks the generalized plane strain using finite strain formulation.
# since we constrain all the nodes against movement and the applied thermal strain
# is very small, the results are the same as small and incremental small strain formulations
#

[GlobalParams]
  displacements = disp_x
  scalar_out_of_plane_strain = scalar_strain_yy
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = lines.e
[]

[Variables]
  [disp_x]
  []
  [temp]
    initial_condition = 580.0
  []
  [scalar_strain_yy]
    order = FIRST
    family = SCALAR
  []
[]

[Functions]
  [temp100]
    type = PiecewiseLinear
    x = '0   1'
    y = '580 680'
  []
  [temp300]
    type = PiecewiseLinear
    x = '0   1'
    y = '580 880'
  []
[]

[Kernels]
  [heat]
    type = Diffusion
    variable = temp
  []
[]

[Modules/TensorMechanics/Master]
  [gps]
    planar_formulation = GENERALIZED_PLANE_STRAIN
    scalar_out_of_plane_strain = scalar_strain_yy
    strain = FINITE
    generate_output = 'strain_xx strain_yy strain_zz stress_xx stress_yy stress_zz'
    eigenstrain_names = eigenstrain
    temperature = temp
  []
[]

[BCs]
  [no_x]
    type = DirichletBC
    boundary = 1000
    value = 0
    variable = disp_x
  []
  [temp100]
    type = FunctionDirichletBC
    variable = temp
    function = temp100
    boundary = 2
  []
  [temp300]
    type = FunctionDirichletBC
    variable = temp
    function = temp300
    boundary = 3
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 3600
    poissons_ratio = 0.2
  []

  [thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 1e-8
    temperature = temp
    stress_free_temperature = 580
    eigenstrain_name = eigenstrain
  []

  [stress]
    type = ComputeFiniteStrainElasticStress
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  line_search = 'none'

  l_max_its = 50
  l_tol = 1e-08
  nl_max_its = 15
  nl_abs_tol = 1e-10
  start_time = 0
  end_time = 1
  num_steps = 1
[]

[Outputs]
  exodus = true
  console = true
[]
