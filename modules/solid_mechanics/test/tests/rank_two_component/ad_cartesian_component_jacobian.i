# A stress component computed by ADRankTwoCartesianComponent is used as a source term in the
# equation of T. The Jacobian is only exact if the derivatives of the component with respect to
# T (through the thermal eigenstrain) and to the displacements are propagated.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [T]
    # Differs from the stress free temperature so that the thermal eigenstrain is not zero
    initial_condition = 1.5
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    strain = SMALL
    use_automatic_differentiation = true
    eigenstrain_names = 'thermal_eigenstrain'
  []
[]

[Kernels]
  [diffusion]
    type = ADDiffusion
    variable = T
  []
  [stress_source]
    type = ADMatBodyForce
    variable = T
    material_property = stress_yy
  []
[]

[Materials]
  # Unit-order data so that all the Jacobian entries have comparable magnitudes
  [elasticity]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1
    poissons_ratio = 0.3
  []
  [thermal_eigenstrain]
    type = ADComputeThermalExpansionEigenstrain
    temperature = T
    thermal_expansion_coeff = 0.1
    stress_free_temperature = 1
    eigenstrain_name = thermal_eigenstrain
  []
  [stress]
    type = ADComputeLinearElasticStress
  []
  [stress_yy]
    type = ADRankTwoCartesianComponent
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
    property_name = stress_yy
  []
[]

[BCs]
  [T_left]
    type = ADDirichletBC
    variable = T
    boundary = left
    value = 1
  []
  [T_right]
    type = ADDirichletBC
    variable = T
    boundary = right
    value = 2
  []
  [disp_x_left]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [disp_y_left]
    type = ADDirichletBC
    variable = disp_y
    boundary = left
    value = 0
  []
  [disp_z_left]
    type = ADDirichletBC
    variable = disp_z
    boundary = left
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]
