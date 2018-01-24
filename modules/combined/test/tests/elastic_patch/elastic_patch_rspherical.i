#
# Patch test for 1D spherical elements
#
# The 1D mesh is pinned at x=0.  The displacement at the outer node is set to
#   3e-3*X where X is the x-coordinate of that node.  That gives a strain of
#   3e-3 for the x, y, and z directions.
#
# Young's modulus is 1e6, and Poisson's ratio is 0.25.  This gives:
#
# Stress xx, yy, zz = E/(1+nu)/(1-2nu)*strain*((1-nu) + nu + nu) = 6000
#

[GlobalParams]
  displacements = 'disp_x'
  temperature = temp
[]

[Problem]
  coord_type = RSPHERICAL
[]

[Mesh]
  file = elastic_patch_rspherical.e
[]

[Functions]
  [./ur]
    type = ParsedFunction
    value = '3e-3*x'
  [../]
[]

[Variables]
  [./disp_x]
  [../]

  [./temp]
    initial_condition = 100.0
  [../]
[]

[AuxVariables]
  [./density]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Modules/TensorMechanics/Master/All]
  strain = SMALL
  incremental = true
  eigenstrain_names = eigenstrain
  add_variables = true
  generate_output = 'stress_xx stress_yy stress_zz'
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temp
  [../]
[]

[AuxKernels]
  [./density]
    type = MaterialRealAux
    property = density
    variable = density
  [../]
[]

[BCs]
  [./ur]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = '1 2'
    function = ur
  [../]

  [./temp]
    type = DirichletBC
    variable = temp
    boundary = 1
    value = 117.56
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.25
  [../]
  [./thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 117.56
    thermal_expansion_coeff = 0.0
    eigenstrain_name = eigenstrain
  [../]
  [./stress]
    type = ComputeStrainIncrementBasedStress
  [../]

  [./heat]
    type = HeatConductionMaterial
    specific_heat = 0.116
    thermal_conductivity = 4.85e-4
  [../]

  [./density]
    type = Density
    density = 0.283
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  exodus = true
[]
