# Generate 1/4 of a 2-ring disk and extrude it by half to obtain
# 1/8 of a 3D tube.  Mirror boundary conditions will exist on the
# cut portions.

[Mesh]
  [disk]
    type = ConcentricCircleMeshGenerator
    num_sectors = 10
    radii = '1.0 1.1 1.2'
    rings = '1 1 1'
    has_outer_square = false
    preserve_volumes = false
    portion = top_right
  []
  [ring]
    type = BlockDeletionGenerator
    input = disk
    block = 1
    new_boundary = 'inner'
  []
  [cylinder]
    type = MeshExtruderGenerator
    input = ring
    extrusion_vector = '0 0 1.5'
    num_layers = 15
    bottom_sideset = 'back'
    top_sideset = 'front'
  []
[]

[Variables]
  [T]
    initial_condition = 300
  []
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Kernels]
  [hc]
    type = HeatConduction
    variable = T
  []
  [TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  []
[]

[BCs]
  [temp_inner]
    type = FunctionNeumannBC
    variable = T
    boundary = 'inner'
    function = surface_source
  []
  [temp_front]
    type = ConvectiveHeatFluxBC
    variable = T
    boundary = 'front'
    T_infinity = 300
    heat_transfer_coefficient = 10
  []
  [temp_outer]
    type = ConvectiveHeatFluxBC
    variable = T
    boundary = 'outer'
    T_infinity = 300
    heat_transfer_coefficient = 10
  []
  # mirror boundary conditions.
  [disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0.0
  []
  [disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0.0
  []
  [disp_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'back'
    value = 0.0
  []
[]

[Materials]
  [cond_inner]
    type = GenericConstantMaterial
    block = 2
    prop_names = thermal_conductivity
    prop_values = 25
  []
  [cond_outer]
    type = GenericConstantMaterial
    block = 3
    prop_names = thermal_conductivity
    prop_values = 100
  []
  [elasticity_tensor_inner]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.1e5
    poissons_ratio = 0.3
    block = 2
  []
  [elasticity_tensor_outer]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 3.1e5
    poissons_ratio = 0.2
    block = 3
  []
  [thermal_strain_inner]
    type = ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 2e-6
    temperature = T
    stress_free_temperature = 300
    eigenstrain_name = eigenstrain_inner
    block = 2
  []
  [thermal_strain_outer]
    type = ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 1e-6
    temperature = T
    stress_free_temperature = 300
    eigenstrain_name = eigenstrain_outer
    block = 3
  []
  [strain_inner] #We use small deformation mechanics
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
    eigenstrain_names = 'eigenstrain_inner'
    block = 2
  []
  [strain_outer] #We use small deformation mechanics
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
    eigenstrain_names = 'eigenstrain_outer'
    block = 3
  []
  [stress] #We use linear elasticity
    type = ComputeLinearElasticStress
  []
[]

[Functions]
  [surface_source]
    type = ParsedFunction
    expression = 'Q_t*pi/2.0/3.0 * cos(pi/3.0*z)'
    symbol_names = 'Q_t'
    symbol_values = heat_source
  []
[]

[Executioner]
  type = Steady

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 101'

  l_max_its = 30
  nl_max_its = 100
  nl_abs_tol = 1e-9
  l_tol = 1e-04
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Controls]
  [stochastic]
    type = SamplerReceiver
  []
[]

[VectorPostprocessors]
  [temp_center]
    type = LineValueSampler
    variable = T
    start_point = '1 0 0'
    end_point = '1.2 0 0'
    num_points = 11
    sort_by = 'x'
  []
  [temp_end]
    type = LineValueSampler
    variable = T
    start_point = '1 0 1.5'
    end_point = '1.2 0 1.5'
    num_points = 11
    sort_by = 'x'
  []
  [dispx_center]
    type = LineValueSampler
    variable = disp_x
    start_point = '1 0 0'
    end_point = '1.2 0 0'
    num_points = 11
    sort_by = 'x'
  []
  [dispx_end]
    type = LineValueSampler
    variable = disp_x
    start_point = '1 0 1.5'
    end_point = '1.2 0 1.5'
    num_points = 11
    sort_by = 'x'
  []
  [dispz_end]
    type = LineValueSampler
    variable = disp_z
    start_point = '1 0 1.5'
    end_point = '1.2 0 1.5'
    num_points = 11
    sort_by = 'x'
  []
[]

[Postprocessors]
  [heat_source]
    type = FunctionValuePostprocessor
    function = 1
    scale_factor = 10000
    execute_on = linear
  []
  [temp_center_inner]
    type = PointValue
    variable = T
    point = '1 0 0'
  []
  [temp_center_outer]
    type = PointValue
    variable = T
    point = '1.2 0 0'
  []
  [temp_end_inner]
    type = PointValue
    variable = T
    point = '1 0 1.5'
  []
  [temp_end_outer]
    type = PointValue
    variable = T
    point = '1.2 0 1.5'
  []
  [dispx_center_inner]
    type = PointValue
    variable = disp_x
    point = '1 0 0'
  []
  [dispx_center_outer]
    type = PointValue
    variable = disp_x
    point = '1.2 0 0'
  []
  [dispx_end_inner]
    type = PointValue
    variable = disp_x
    point = '1 0 1.5'
  []
  [dispx_end_outer]
    type = PointValue
    variable = disp_x
    point = '1.2 0 1.5'
  []
  [dispz_inner]
    type = PointValue
    variable = disp_z
    point = '1 0 1.5'
  []
  [dispz_outer]
    type = PointValue
    variable = disp_z
    point = '1.2 0 1.5'
  []
[]

[Outputs]
  exodus = false
  csv = false
[]
