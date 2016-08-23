# The purpose of this set of tests is to check the values computed
# by the RadialDisplacementAux AuxKernel. They should match the
# radial component of the displacment for a cylindrical or spherical
# model.
# This particular model is of a cylinder subjected to uniform thermal
# expansion represented using a 3D Cartesian model.

[Mesh]
  type = FileMesh
  file = cylinder_sector_3d.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  order = SECOND
  family = LAGRANGE
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
  [./temp]
  [../]
  [./rad_disp]
  [../]
[]

[Functions]
  [./temperature_load]
    type = ParsedFunction
    value = t+300.0
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
  [../]
[]

[AuxKernels]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = temperature_load
    use_displaced_mesh = false
  [../]
  [./raddispaux]
    type = RadialDisplacementCylinderAux
    variable = rad_disp
    origin = '0 0 0'
    axis_vector = '0 0 1'
  [../]
[]

[BCs]
  [./x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./y]
    type = DirichletBC
    variable = disp_y
    boundary = 2
    value = 0.0
  [../]
  [./z]
    type = DirichletBC
    variable = disp_z
    boundary = '3 4'
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 1
    youngs_modulus = 2.1e5
    poissons_ratio = 0.3
  [../]
  [./finite_strain]
    type = ComputeFiniteStrain
    block = 1
  [../]
  [./small_stress]
    type = ComputeFiniteStrainElasticStress
    block = 1
  [../]
  [./thermal_expansion]
    type = ComputeThermalExpansionEigenStrain
    block = 1
    stress_free_reference_temperature = 300
    thermal_expansion_coeff = 1.3e-5
    temperature = temp
    incremental_form = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '51'

  line_search = 'none'

  l_max_its = 50
  nl_max_its = 50
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10

  start_time = 0.0
  end_time = 1
  dt = 1
  dtmin = 1
[]

[Outputs]
 csv = true
 exodus = true
[]

#[Postprocessors]
#  [./strain_xx]
#    type = SideAverageValue
#    variable =
#    block = 0
#  [../]
#[]
