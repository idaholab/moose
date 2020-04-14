# Simple test to check for use of AxisymmetricCenterlineAverageValue with
# volumetric_locking_correction activated in a tensor mechanics simulation

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[GlobalParams]
  displacements = 'disp_r disp_z'
  volumetric_locking_correction = true
[]

[Problem]
  coord_type = RZ
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    use_automatic_differentiation = true
  [../]
[]

[AuxVariables]
  [./temperature]
    initial_condition = 298.0
  [../]
[]

[BCs]
  [./symmetry_x]
    type = ADDirichletBC
    variable = disp_r
    value = 0
    boundary = left
  [../]
  [./roller_z]
    type = ADDirichletBC
    variable = disp_z
    value = 0
    boundary = bottom
  [../]
  [./top_load]
    type = ADFunctionDirichletBC
    variable = disp_z
    function = -0.01*t
    boundary = top
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.3
  [../]
  [./_elastic_strain]
    type = ADComputeFiniteStrainElasticStress
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  line_search = 'none'

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  nl_max_its = 15

  l_tol = 1e-6
  l_max_its = 50

  start_time = 0.0
  end_time = 0.3
  dt = 0.1
[]

[Postprocessors]
  [./center_temperature]
    type = AxisymmetricCenterlineAverageValue
    variable = temperature
    boundary = left
  [../]
[]

[Outputs]
  csv = true
  perf_graph = true
[]
