# This tests the ability of the ComputeVolumetricEigenstrain material
# to compute an eigenstrain tensor that results in a solution that exactly
# recovers the specified volumetric expansion.
# This model applies volumetric strain that ramps from 0 to 2 to a unit cube
# and computes the final volume, which should be exactly 3.  Note that the default
# TaylorExpansion option for decomposition_method gives a small (~4%) error
# with this very large incremental strain, but decomposition_method=EigenSolution
# gives the exact solution.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[AuxVariables]
  [./volumetric_strain]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/Master]
  [./master]
    strain = FINITE
    eigenstrain_names = eigenstrain
    decomposition_method = EigenSolution #Necessary for exact solution
  [../]
[]

[AuxKernels]
  [./volumetric_strain]
    type = RankTwoScalarAux
    scalar_type = VolumetricStrain
    rank_two_tensor = total_strain
    variable = volumetric_strain
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./bottom]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./back]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./finite_strain_stress]
    type = ComputeFiniteStrainElasticStress
  [../]
  [./volumetric_eigenstrain]
    type = ComputeVolumetricEigenstrain
    volumetric_materials = volumetric_change
    eigenstrain_name = eigenstrain
    args = ''
  [../]
  [./volumetric_change]
    type = GenericFunctionMaterial
    prop_names = volumetric_change
    prop_values = t
  [../]
[]

[Postprocessors]
  [./vol]
    type = VolumePostprocessor
    use_displaced_mesh = true
    execute_on = 'initial timestep_end'
  [../]
  [./volumetric_strain]
    type = ElementalVariableValue
    variable = volumetric_strain
    elementid = 0
  [../]
  [./disp_right]
    type = NodalExtremeValue
    variable = disp_x
    boundary = right
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  l_max_its = 100
  l_tol = 1e-4
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-12

  start_time = 0.0
  end_time = 2.0
  dt = 1.0
[]

[Outputs]
  csv = true
[]
