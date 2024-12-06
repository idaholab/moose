[Debug]
  show_material_props = true
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  displacements = 'disp_x disp_y disp_z'
[]

[AuxVariables]
  [disp_y]
    initial_condition = 0
  []
  [disp_x]
    initial_condition = 0
  []
  [disp_z]
    initial_condition = 0
  []
[]

[AuxKernels]
  # The applied displacements will cause the max eigenvector to change directions.
  # At t=5, the body undergoes simple shear, producing a nonsymmetric deformation gradient.
  [disp_x]
    execute_on = 'TIMESTEP_BEGIN'
    type = ParsedAux
    variable = disp_x
    use_xyzt = true
    expression = "if(t<4.1,4e-1*x*t,x)"
  []
  [disp_y]
    execute_on = 'TIMESTEP_BEGIN'
    type = ParsedAux
    variable = disp_y
    use_xyzt = true
    expression = "if(t<4.1,3e-1*y*t^2,1e-1*y*t+1e-1*x*t)"
  []
  [disp_z]
    execute_on = 'TIMESTEP_BEGIN'
    type = ParsedAux
    variable = disp_z
    use_xyzt = true
    expression = "if(t<4.1,1e-1*z*t^3,z)"
  []
[]

[Materials]
  [compute_strain]
    type = ComputeLagrangianStrain
    displacements = 'disp_x disp_y disp_z'
    large_kinematics = true
  []
  [nonAD_strain]
    type = RankTwoTensorMaterialADConverter
    reg_props_in = mechanical_strain
    ad_props_out = AD_mechanical_strain
  []
  [eig_decomp]
    type = ADEigenDecompositionMaterial
    rank_two_tensor = AD_mechanical_strain
    outputs = exodus
    output_properties = "max_eigen_vector mid_eigen_vector min_eigen_vector max_eigen_value "
                        "mid_eigen_value min_eigen_value"
  []
  [nonADeig_decomp]
    type = EigenDecompositionMaterial
    rank_two_tensor = mechanical_strain
    base_name = nonAD
    outputs = exodus
    output_properties = "nonAD_max_eigen_vector nonAD_mid_eigen_vector nonAD_min_eigen_vector "
                        "nonAD_max_eigen_value nonAD_mid_eigen_value nonAD_min_eigen_value"
  []

  [non_symmetric_eig_decomp_error]
    type = EigenDecompositionMaterial
    rank_two_tensor = deformation_gradient
    base_name = nonSym
  []
[]

[BCs]
[]

[Executioner]
  type = Transient
  solve_type = LINEAR
  dt = 1
  end_time = 5
[]

[Postprocessors]
  [sxx]
    type = MaterialTensorAverage
    rank_two_tensor = mechanical_strain
    index_i = 0
    index_j = 0
    execute_on = 'TIMESTEP_END'
  []
  [sxy]
    type = MaterialTensorAverage
    rank_two_tensor = mechanical_strain
    index_i = 0
    index_j = 1
    execute_on = 'TIMESTEP_END'
  []
  [syy]
    type = MaterialTensorAverage
    rank_two_tensor = mechanical_strain
    index_i = 1
    index_j = 1
  []
  [szz]
    type = MaterialTensorAverage
    rank_two_tensor = mechanical_strain
    index_i = 1
    index_j = 0
  []
  [AD_eigval_max]
    type = ADElementAverageMaterialProperty
    mat_prop = max_eigen_value
  []
  [AD_eigval_mid]
    type = ADElementAverageMaterialProperty
    mat_prop = mid_eigen_value
  []
  [AD_eigval_min]
    type = ADElementAverageMaterialProperty
    mat_prop = min_eigen_value
  []
  [nonAD_eigval_max]
    type = ElementAverageMaterialProperty
    mat_prop = nonAD_max_eigen_value
  []
  [nonAD_eigval_mid]
    type = ElementAverageMaterialProperty
    mat_prop = nonAD_mid_eigen_value
  []
  [nonAD_eigval_min]
    type = ElementAverageMaterialProperty
    mat_prop = nonAD_min_eigen_value
  []
[]

[Outputs]
  exodus = true
[]
