[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [vel_x]
  []
  [vel_y]
  []
[]

[Kernels]
  [sdx]
    type = TotalLagrangianStressDivergence
    variable = disp_x
    component = 0
  []
  [sdy]
    type = TotalLagrangianStressDivergence
    variable = disp_y
    component = 1
  []

  [ifx]
    type = InertialForce
    variable = disp_x
    density = 1
    use_displaced_mesh = false
  []
  [ify]
    type = InertialForce
    variable = disp_y
    density = 1
    use_displaced_mesh = false
  []
[]

[AuxKernels]
  [vel_x]
    type = TestNewmarkTI
    variable = vel_x
    displacement = disp_x
    first = true
    execute_on = 'LINEAR TIMESTEP_BEGIN TIMESTEP_END'
  []
  [vel_y]
    type = TestNewmarkTI
    variable = vel_y
    displacement = disp_y
    first = true
    execute_on = 'LINEAR TIMESTEP_BEGIN TIMESTEP_END'
  []
[]

[Materials]
  [elasticity_slug]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1
    poissons_ratio = 0.25
  []
  [lagrangian_strain]
    type = ComputeLagrangianStrain
  []
  [stress]
    type = ComputeLagrangianLinearElasticStress
  []
  [density]
    type = GenericConstantMaterial
    prop_names  = density
    prop_values = 1
  []
[]

[BCs]
  [bc_x]
    type = NeumannBC
    variable = disp_x
    boundary = 'top bottom left right'
    value = 0
  []
  [bc_y]
    type = NeumannBC
    variable = disp_y
    boundary = 'top bottom left right'
    value = 0
  []
[]

[ICs]
  [current]
    type = ConstantIC
    variable = disp_x
    value = 0
    state = CURRENT
  []
  [old]
    type = ConstantIC
    variable = disp_x
    value = -1
    state = OLD
  []
[]

[Postprocessors]
  [disp_x]
    type = ElementAverageValue
    variable = disp_x
  []
  [vel_x]
    type = ElementAverageValue
    variable = vel_x
  []
[]

[Executioner]
  type = Transient
  [TimeIntegrator]
    type = CentralDifference
    solve_type = lumped
  []
  solve_type = LINEAR
  dt = 1
  end_time = 3
[]

[Outputs]
  exodus = true
  csv = true
[]
