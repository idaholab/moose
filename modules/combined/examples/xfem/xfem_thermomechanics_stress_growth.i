# This is a demonstration of a simple thermomechanics simulation using
# XFEM in which a single crack propagates based on a principal stress
# criterion.
#
# The top and bottom of the plate are fixed in the y direction, and the
# top of the plate is cooled down over time. The thermal contraction
# causes tensile stresses, which lead to crack propagation. The crack
# propagates in a curved path because of the changinging nature of
# the thermal gradient as a result of the crack. There is no heat
# conduction across the crack as soon as it forms.

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 11
  ny = 11
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  elem_type = QUAD4
[]

[Variables]
  # Solve for the temperature and the displacements
  # Displacements are not specified because the TensorMechanics/Master Action sets them up
  [./temp]
    initial_condition = 300
  [../]
[]

[XFEM]
  geometric_cut_userobjects = 'line_seg_cut_uo'
  qrule = volfrac
  output_cut_plane = true
[]

[UserObjects]
  [./line_seg_cut_uo]
    type = LineSegmentCutUserObject
    cut_data = '1.0  0.5  0.8  0.5'
    time_start_cut = 0.0
    time_end_cut = 0.0
  [../]
  [./xfem_marker_uo]
    type = XFEMRankTwoTensorMarkerUserObject
    execute_on = timestep_end
    tensor = stress
    scalar_type = MaxPrincipal
    threshold = 5e+1
    average = true
  [../]
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    strain = FINITE
    planar_formulation = plane_strain
    add_variables = true
    eigenstrain_names = eigenstrain
  [../]
[]

[Kernels]
  [./htcond]
    type = HeatConduction
    variable = temp
  [../]
[]

[BCs]
  [./bottomx]
    type = DirichletBC
    boundary = bottom
    variable = disp_x
    value = 0.0
  [../]
  [./bottomy]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0.0
  [../]
  [./topx]
    type = DirichletBC
    boundary = top
    variable = disp_x
    value = 0.0
  [../]
  [./topy]
    type = DirichletBC
    boundary = top
    variable = disp_y
    value = 0.0
  [../]
  [./topt]
    type = FunctionDirichletBC
    boundary = top
    variable = temp
    function = 273-t*27.3
  [../]
  [./bott]
    type = FunctionDirichletBC
    boundary = bottom
    variable = temp
    function = 273
#    value = 273.0
  [../]
[]

[Materials]
  [./thcond]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '5e-6'
  [../]

  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]

  [./_elastic_strain]
    type = ComputeFiniteStrainElasticStress
  [../]

  [./thermal_strain]
    type= ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 10e-6
    temperature = temp
    stress_free_temperature = 273
    eigenstrain_name = eigenstrain
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      8'

  line_search = 'none'

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]

# controls for linear iterations
  l_max_its = 100
  l_tol = 1e-2

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-14
  nl_abs_tol = 1e-9

# time control
  start_time = 0.0
  dt = 1.0
  end_time = 10.0

  max_xfem_update = 5
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
  [./console]
    type = Console
    output_linear = true
  [../]
[]
