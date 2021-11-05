#
# Added subdomains and subdomain-specific properties
# https://mooseframework.inl.gov/modules/tensor_mechanics/tutorials/introduction/step03.html
#

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [generated]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 20
    xmin = -0.25
    xmax = 0.25
    ymax = 5
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    input = generated
    block_id = 1
    bottom_left = '-0.25 0 0'
    top_right = '0 5 0'
  []
  [block2]
    type = SubdomainBoundingBoxGenerator
    input = block1
    block_id = 2
    bottom_left = '0 0 0'
    top_right = '0.25 5 0'
  []

  # select a single node in the center of the bottom boundary
  [pin]
    type = ExtraNodesetGenerator
    input = block2
    new_boundary = pin
    coord = '0 0 0'
  []
[]

[AuxVariables]
  [T]
  []
[]

[AuxKernels]
  [temperature_ramp]
    type = FunctionAux
    execute_on = TIMESTEP_BEGIN
    variable = T
    function = 300+5*t
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    automatic_eigenstrain_names = true
    generate_output = 'vonmises_stress'
  []
[]

[BCs]
  [pin_x]
    type = DirichletBC
    variable = disp_x
    boundary = pin
    value = 0
  []
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e9
    poissons_ratio = 0.3
  []
  [expansion1]
    type = ComputeThermalExpansionEigenstrain
    temperature = T
    thermal_expansion_coeff = 0.001
    stress_free_temperature = 300
    eigenstrain_name = thermal_expansion
    block = 1
  []
  [expansion2]
    type = ComputeThermalExpansionEigenstrain
    temperature = T
    thermal_expansion_coeff = 0.002
    stress_free_temperature = 300
    eigenstrain_name = thermal_expansion
    block = 2
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  end_time = 5
  dt = 1
[]

[Outputs]
  exodus = true
[]
