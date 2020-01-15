# This test is used to verify that the pure test object (TestCrackCounter)
# is correctly using the API for extracting the crack_tip_origin_direction_map
# from XFEM.  The map contains information of the location of all the crack tips
# computed by XFEM.  The TestCrackCounter postprocessor simply returns the
# number of elements in the map which corresponds to the number of cracks.
#
# In this test case 4 prescribed cracks are applied.  Therefore, the
# TestCrackCounter postprocessor returns a value of 4.

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[XFEM]
  qrule = volfrac
  output_cut_plane = true
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

[UserObjects]
  [./line_seg_cut_uo]
    type = LineSegmentCutUserObject
    cut_data = '1.0  0.5  0.7  0.5'
    time_start_cut = 0.0
    time_end_cut = 0.0
  [../]
  [./line_seg_cut_uo2]
    type = LineSegmentCutUserObject
    cut_data = '0.0  0.5  0.3  0.5'
    time_start_cut = 0.0
    time_end_cut = 0.0
  [../]
  [./line_seg_cut_uo3]
    type = LineSegmentCutUserObject
    cut_data = '0.5  0.0  0.5  0.3'
    time_start_cut = 0.0
    time_end_cut = 0.0
  [../]
  [./line_seg_cut_uo4]
    type = LineSegmentCutUserObject
    cut_data = '0.5  1.0  0.5  0.7'
    time_start_cut = 0.0
    time_end_cut = 0.0
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    planar_formulation = plane_strain
    add_variables = true
  [../]
[]

[Functions]
  [./pull]
    type = PiecewiseLinear
    x='0  50   100'
    y='0  0.02 0.1'
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
    type = FunctionDirichletBC
    boundary = top
    variable = disp_y
    function = pull
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    block = 0
  [../]

  [./_elastic_strain]
    type = ComputeFiniteStrainElasticStress
    block = 0
  [../]
[]

[Postprocessors]
  [./number_of_cracks]
    type = TestCrackCounter
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      8'

  line_search = 'none'

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
  end_time = 1.0
  num_steps = 5000

  max_xfem_update = 1
[]

[Outputs]
  csv = true
[]
