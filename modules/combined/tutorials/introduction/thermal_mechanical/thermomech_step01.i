#
# Single block coupled thermal/mechanical
# https://mooseframework.inl.gov/modules/combined/tutorials/introduction/thermoech_step01.html
#

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [generated]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmax = 2
    ymax = 1
  []
  [pin]
    type = ExtraNodesetGenerator
    input = generated
    new_boundary = pin
    coord = '0 0 0'
  []
[]

[Variables]
  [T]
    initial_condition = 300.0
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = T
  []
  [time_derivative]
    type = HeatConductionTimeDerivative
    variable = T
  []
  [heat_source]
    type = HeatSource
    variable = T
    value = 5e4
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
    automatic_eigenstrain_names = true
    generate_output = 'vonmises_stress'
  []
[]

[Materials]
  [thermal]
    type = HeatConductionMaterial
    thermal_conductivity = 45.0
    specific_heat = 0.5
  []
  [density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = 8000.0
  []
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
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
[]

[BCs]
  [t_left]
    type = DirichletBC
    variable = T
    value = 300
    boundary = 'left'
  []
  [t_right]
    type = FunctionDirichletBC
    variable = T
    function = '300+5*t'
    boundary = 'right'
  []
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

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  end_time = 5
  dt = 1
[]

[Outputs]
  exodus = true
[]
