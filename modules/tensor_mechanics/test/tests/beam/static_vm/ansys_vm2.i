# This is a reproduction of test number 2 of ANSYS apdl verification manual.

# This test checks for the deformation at the center of a beam with simply
# supported boundary conditions and a uniform load w = 10,000 lb/ft.

#    |||||||||      def.      ||||||||
#    *---*---*---*---*---*---*---*---*
#            /\              /\
#           ///              oo
#        a           l            a
#     <-----> <--------------> <----->
#

# l = 240 in, a = 120 in, A = 50.65 in^2, Iz = 7892 in^2
# E = 30e6 psi
# Solution deflection: 0.182 in. (dispz_5: -1.824633e-01)

[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 8
    xmin = 0.0
    xmax = 480.0
  []
  [cnode]
    type = ExtraNodesetGenerator
    coord = '0.0'
    new_boundary = 'one'
    input = generated_mesh
  []
  [cnode1]
    type = ExtraNodesetGenerator
    coord = '60.0'
    new_boundary = 'two'
    input = cnode
  []
  [cnode2]
    type = ExtraNodesetGenerator
    coord = '420.0'
    new_boundary = 'eight'
    input = cnode1
  []
  [cnode3]
    type = ExtraNodesetGenerator
    coord = '480.0'
    new_boundary = 'nine'
    input = cnode2
  []

  [cnode4]
    type = ExtraNodesetGenerator
    coord = '120.0'
    new_boundary = 'BC1'
    input = cnode3
  []

  [cnode5]
    type = ExtraNodesetGenerator
    coord = '360.0'
    new_boundary = 'BC2'
    input = cnode4
  []
[]

[Modules/TensorMechanics/LineElementMaster]
  [./all]
    add_variables = true
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    # Geometry parameters
    area = 50.65
    Ay = 0.0
    Az = 0.0
    Iy = 7892.0
    Iz = 7892.0
    y_orientation = '0 1.0 0.0'
  [../]
[]

[Materials]
  [./elasticity]
    type = ComputeElasticityBeam
    youngs_modulus = 30.0e6
    # poissons_ratio = -0.9998699638
      poissons_ratio = 0.33
    # poissons_ratio = 0.3
    shear_coefficient = 0.85
    block = 0
  [../]
  [./stress]
    type = ComputeBeamResultants
    block = 0
  [../]
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = 'BC1'
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = 'BC1'
    value = 0.0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = 'BC1'
    value = 0.0
  [../]

  [./fixy2]
    type = DirichletBC
    variable = disp_y
    boundary = 'BC2'
    value = 0.0
  [../]
  [./fixz2]
    type = DirichletBC
    variable = disp_z
    boundary = 'BC2'
    value = 0.0
  [../]
[]

[Functions]
  [./force_50e3]
    type = PiecewiseLinear
    x = '0.0 10.0'
    y = '0.0 50000.0'
  [../]
  [./force_25e3]
    type = PiecewiseLinear
    x = '0.0 10.0'
    y = '0.0 25000.0'
  [../]
[]

[NodalKernels]
  [./force_z2]
    type = UserForcingFunctionNodalKernel
    variable = disp_z
    boundary = 'two'
    function = force_50e3
  [../]
  [./force_z8]
    type = UserForcingFunctionNodalKernel
    variable = disp_z
    boundary = 'eight'
    function = force_50e3
  [../]

  [./force_z1]
    type = UserForcingFunctionNodalKernel
    variable = disp_z
    boundary = 'one'
    function = force_25e3
  [../]
  [./force_z9]
    type =  UserForcingFunctionNodalKernel
    variable = disp_z
    boundary = 'nine'
    function = force_25e3
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]
[Executioner]
  type = Transient
  solve_type = JFNK
  line_search = 'none'
  nl_max_its = 15
  nl_rel_tol = 1e-06
  nl_abs_tol = 1e-06

  dt = 1.0
  dtmin = 0.001
  end_time = 10
[]

[Postprocessors]
  [./disp_z1]
    type = PointValue
    point = '0.0 0.0 0.0'
    variable = disp_z
  [../]
  [./disp_x1]
    type = PointValue
    point = '0.0 0.0 0.0'
    variable = disp_x
  [../]
  [./disp_z2]
    type = PointValue
    point = '60.0 0.0 0.0'
    variable = disp_z
  [../]
  [./disp_zBC1]
    type = PointValue
    point = '120.0 0.0 0.0'
    variable = disp_z
  [../]
  [./disp_z5]
    type = PointValue
    point = '240.0 0.0 0.0'
    variable = disp_z
  [../]
  [./disp_zBC2]
    type = PointValue
    point = '360.0 0.0 0.0'
    variable = disp_z
  [../]
  [./disp_xBC2]
    type = PointValue
    point = '360.0 0.0 0.0'
    variable = disp_x
  [../]
  [./disp_z8]
    type = PointValue
    point = '420.0 0.0 0.0'
    variable = disp_z
  [../]
  [./disp_z9]
    type = PointValue
    point = '480.0 0.0 0.0'
    variable = disp_z
  [../]
[]
[Debug]
 show_var_residual_norms = true
[]

[Outputs]
  csv = true
  exodus = false
[]
