# This test ensures that the reported volumetric strain for a cube with
# mechanically imposed displacements (through Dirichlet BCs) exactly
# matches that from a version of this test that experiences the same
# defomation, but due to imposed eigenstrains.

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

[Functions]
  [pres_disp]
    type = PiecewiseLinear
    # These values are taken from the displacements in the eigenstrain
    # version of this test. The volume of the cube (which starts out as
    # a 1x1x1 cube) is (1 + disp)^3. At time 2, this is
    # (1.44224957030741)^3, which is 3.0.
    xy_data = '0 0
               1 0.25992104989487
               2 0.44224957030741'
  []
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
  [./right]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = pres_disp
  [../]
  [./top]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = pres_disp
  [../]
  [./front]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = pres_disp
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
