[GlobalParams]
  order = SECOND
  displacements = 'disp_x disp_y'
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  patch_update_strategy = iteration
  [./gen]
    type = FileMeshGenerator
    file = mesh.e
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./temp]
    initial_condition = 501
  [../]
[]

[AuxVariables]
  [./density_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Modules/TensorMechanics/Master]
  [./finite]
    strain = FINITE
  [../]
[]

[Kernels]
  [./gravity]
    type = Gravity
    variable = disp_y
    value = -9.81
  [../]
  [./heat]
    type = MatDiffusion
    variable = temp
    diffusivity = 1
  [../]
  [./heat_ie]
    type = TimeDerivative
    variable = temp
  [../]
[]

[AuxKernels]
  [./conductance]
    type = MaterialRealAux
    property = density
    variable = density_aux
    boundary = inner_surface
  [../]
[]

[ThermalContact]
  [./thermal_contact]
    type = GapHeatTransfer
    variable = temp
    master = outer_interior
    secondary = inner_surface
    emissivity_master = 0
    emissivity_secondary = 0
    quadrature = true
  [../]
[]

[BCs]
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'centerline'
    value = 0.0
  [../]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'centerline outer_exterior'
    value = 0.0
  [../]
  [./temp]
    type = FunctionDirichletBC
    boundary = outer_exterior
    variable = temp
    function = '500 + t'
  [../]
[]

[Materials]
  [./density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '1'
  [../]

  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e11
    poissons_ratio = 0.3
  [../]

  [./inner_elastic_stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'inner_creep'
    block = inner
    outputs = all
  [../]

  [./inner_creep]
    type = PowerLawCreepExceptionTest
    coefficient = 10e-22
    n_exponent = 2
    activation_energy = 0
    block = inner
  [../]

  [./outer_stressstress]
    type = ComputeFiniteStrainElasticStress
    block = outer
  [../]
[]

[Executioner]
  type = Transient
  petsc_options = ' -snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = none

  nl_abs_tol = 1e-7
  l_max_its = 20

  num_steps = 1
  dt = 1
  dtmin = .1
[]

[Outputs]
  exodus = true
[]
