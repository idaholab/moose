# This test involves only thermal expansion strains on a 2x2x2 cube of approximate
# steel material.  An initial temperature of 25 degrees C is given for the material,
# and an auxkernel is used to calculate the temperature in the entire cube to
# raise the temperature each time step.  After the first timestep,in which the
# temperature jumps, the temperature increases by 6.25C each timestep.
# The thermal strain increment should therefore be
#     6.25 C * 1.3e-5 1/C = 8.125e-5 m/m.

# This test is also designed to be used to identify problems with restart files

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 5
  xmin = 0.0
  xmax = 0.5
  ymin = 0.0
  ymax = 0.150080
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [./temp]
  [../]
  [./axial_strain]
    order = FIRST
    family = MONOMIAL
  [../]
[]

[Functions]
  [./temperature_load]
    type = ParsedFunction
    expression = t*(1000.0)+300.0
  [../]
[]

[Physics]
  [./SolidMechanics]
    [./QuasiStatic]
      [./all]
        strain = SMALL
        incremental = true
        add_variables = true
        eigenstrain_names = eigenstrain
      [../]
    [../]
  [../]
[]

[AuxKernels]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = temperature_load
  [../]
  [./axial_strain]
    type = RankTwoAux
    variable = axial_strain
    rank_two_tensor = total_strain
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./x_bot]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.1e5
    poissons_ratio = 0.3
  [../]
  [./small_stress]
    type = ComputeFiniteStrainElasticStress
  [../]
  [./thermal_expansion_strain]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 298
    thermal_expansion_coeff = 1.3e-5
    temperature = temp
    eigenstrain_name = eigenstrain
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 50
  nl_max_its = 50
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9

  start_time = 0.0
  end_time = 0.075
  dt = 0.0125
  dtmin = 0.0001
[]

[Outputs]
  exodus = true
[]

[VectorPostprocessors]
  [./axial_str]
    type = LineValueSampler
    warn_discontinuous_face_values = false
    start_point = '0.5 0.0 0.0'
    end_point = '0.5 0.150080 0.0'
    variable = axial_strain
    num_points = 11
    sort_by = 'id'
  [../]
[]

[Postprocessors]
  [./end_disp]
    type = PointValue
    variable = disp_y
    point = '0.5 0.150080 0.0'
  [../]
[]
