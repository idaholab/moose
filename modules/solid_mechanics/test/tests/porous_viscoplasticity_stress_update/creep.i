end_time = 1
dt = ${fparse end_time / 10}
strain = 1e-5
AD = AD
use_ad = true

[GlobalParams]
  displacements = 'disp_x disp_y'
  absolute_value_vector_tags = 'ref'
[]


[Problem]
  extra_tag_vectors = 'ref'
[]

[Mesh]
  [gen]
    type = ExamplePatchMeshGenerator
    dim = 2
  []
[]

[Physics/SolidMechanics/QuasiStatic/All]
  strain = FINITE
  add_variables = true
  generate_output = 'strain_xx strain_yy strain_xy hydrostatic_stress vonmises_stress'
  use_automatic_differentiation = ${use_ad}
[]

[Functions]
  [pull]
    type = PiecewiseLinear
    x = '0 ${end_time}'
    y = '0 ${strain}'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ${AD}ComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.3
  []
  [stress]
    type = ${AD}ComputeMultipleInelasticStress
    inelastic_models = creep
    outputs = all
  []
  [porosity]
    type = ${AD}PorosityFromStrain
    initial_porosity = 0.1
    inelastic_strain = 'combined_inelastic_strain'
    outputs = all
  []
  [creep]
    type = ${AD}PowerLawCreepStressUpdate
    activation_energy = 4e4
    temperature = 1200
    coefficient = 1e-18
    gas_constant = 1.987
    n_exponent = 3
    outputs = all
    absolute_tolerance = 1e-15
  []
[]

[BCs]
  [no_disp_x]
    type = ${AD}DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [no_disp_y]
    type = ${AD}DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [pull_disp_y]
    type = ${AD}FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = pull
  []
[]

[Convergence]
  [disp_ref_check]
    type = ReferenceResidualConvergence
    reference_vector = 'ref'
    converge_on = 'disp_x disp_y'
    group_variables = 'disp_x disp_y'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nonlinear_convergence = disp_ref_check
  dt = ${dt}
  end_time = ${end_time}
[]

[Postprocessors]
  [disp_x]
    type = SideAverageValue
    variable = disp_x
    boundary = right
  []
  [disp_y]
    type = SideAverageValue
    variable = disp_y
    boundary = top
  []
  [avg_hydro]
    type = ElementAverageValue
    variable = hydrostatic_stress
  []
  [avg_vonmises]
    type = ElementAverageValue
    variable = vonmises_stress
  []
  [dt]
    type = TimestepSize
    outputs = console
  []
  [eff_creep_strain]
    type = ElementAverageValue
    variable = effective_creep_strain
  []
  [porosity]
    type = ElementAverageValue
    variable = porosity
  []
[]

[Outputs]
  csv = true
[]
