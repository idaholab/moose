# apply repeated stretches in z direction, and smaller stretches along the y direction, and compression along x direction
# Both return to the plane and edge (lode angle = 30deg, ie 010100) are experienced.
#
# It is checked that the yield functions are less than their tolerance values
# It is checked that the cohesion hardens correctly

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
[]


[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
[]


[BCs]
  [./x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'front back'
    function = '-1E-6*x*t'
  [../]
  [./y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'front back'
    function = '0.05E-6*y*t'
  [../]
  [./z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'front back'
    function = '1E-6*z*t'
  [../]
[]

[Functions]
  [./should_be_zero_fcn]
    type = ParsedFunction
    expression = 'if((a<1E-5)&(b<1E-5)&(c<1E-5)&(d<1E-5)&(g<1E-5)&(h<1E-5),0,abs(a)+abs(b)+abs(c)+abs(d)+abs(g)+abs(h))'
    symbol_names = 'a b c d g h'
    symbol_values = 'f0 f1 f2 f3 f4 f5'
  [../]
  [./coh_analytic]
    type = ParsedFunction
    expression = '20-10*exp(-1E6*intnl)'
    symbol_names = intnl
    symbol_values = internal
  [../]
  [./coh_from_yieldfcns]
    type = ParsedFunction
    expression = '(f0+f1-(sxx+syy)*sin(phi))/(-2)/cos(phi)'
    symbol_names = 'f0 f1 sxx syy phi'
    symbol_values = 'f0 f1 s_xx s_yy 0.8726646'
  [../]
  [./should_be_zero_coh]
    type = ParsedFunction
    expression = 'if(abs(a-b)<1E-6,0,1E6*abs(a-b))'
    symbol_names = 'a b'
    symbol_values = 'Coh_analytic Coh_moose'
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./mc_int]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./yield_fcn0]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./yield_fcn1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./yield_fcn2]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./yield_fcn3]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./yield_fcn4]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./yield_fcn5]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  [../]
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
  [../]
  [./stress_xz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xz
    index_i = 0
    index_j = 2
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  [../]
  [./stress_yz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yz
    index_i = 1
    index_j = 2
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  [../]
  [./mc_int_auxk]
    type = MaterialStdVectorAux
    index = 0
    property = plastic_internal_parameter
    variable = mc_int
  [../]
  [./yield_fcn0]
    type = MaterialStdVectorAux
    index = 0
    property = plastic_yield_function
    variable = yield_fcn0
  [../]
  [./yield_fcn1]
    type = MaterialStdVectorAux
    index = 1
    property = plastic_yield_function
    variable = yield_fcn1
  [../]
  [./yield_fcn2]
    type = MaterialStdVectorAux
    index = 2
    property = plastic_yield_function
    variable = yield_fcn2
  [../]
  [./yield_fcn3]
    type = MaterialStdVectorAux
    index = 3
    property = plastic_yield_function
    variable = yield_fcn3
  [../]
  [./yield_fcn4]
    type = MaterialStdVectorAux
    index = 4
    property = plastic_yield_function
    variable = yield_fcn4
  [../]
  [./yield_fcn5]
    type = MaterialStdVectorAux
    index = 5
    property = plastic_yield_function
    variable = yield_fcn5
  [../]
[]

[Postprocessors]
  [./s_xx]
    type = PointValue
    point = '0 0 0'
    variable = stress_xx
  [../]
  [./s_xy]
    type = PointValue
    point = '0 0 0'
    variable = stress_xy
  [../]
  [./s_xz]
    type = PointValue
    point = '0 0 0'
    variable = stress_xz
  [../]
  [./s_yy]
    type = PointValue
    point = '0 0 0'
    variable = stress_yy
  [../]
  [./s_yz]
    type = PointValue
    point = '0 0 0'
    variable = stress_yz
  [../]
  [./s_zz]
    type = PointValue
    point = '0 0 0'
    variable = stress_zz
  [../]
  [./internal]
    type = PointValue
    point = '0 0 0'
    variable = mc_int
  [../]
  [./f0]
    type = PointValue
    point = '0 0 0'
    variable = yield_fcn0
  [../]
  [./f1]
    type = PointValue
    point = '0 0 0'
    variable = yield_fcn1
  [../]
  [./f2]
    type = PointValue
    point = '0 0 0'
    variable = yield_fcn2
  [../]
  [./f3]
    type = PointValue
    point = '0 0 0'
    variable = yield_fcn3
  [../]
  [./f4]
    type = PointValue
    point = '0 0 0'
    variable = yield_fcn4
  [../]
  [./f5]
   type = PointValue
    point = '0 0 0'
    variable = yield_fcn5
  [../]
  [./yfcns_should_be_zero]
    type = FunctionValuePostprocessor
    function = should_be_zero_fcn
  [../]
  [./Coh_analytic]
    type = FunctionValuePostprocessor
    function = coh_analytic
  [../]
  [./Coh_moose]
    type = FunctionValuePostprocessor
    function = coh_from_yieldfcns
  [../]
  [./cohesion_difference_should_be_zero]
    type = FunctionValuePostprocessor
    function = should_be_zero_coh
  [../]
[]

[UserObjects]
  [./mc_coh]
    type = TensorMechanicsHardeningExponential
    value_0 = 10
    value_residual = 20
    rate = 1E6
  [../]
  [./mc_phi]
    type = TensorMechanicsHardeningConstant
    value = 0.8726646
  [../]
  [./mc_psi]
    type = TensorMechanicsHardeningConstant
    value = 1 #0.8726646 # 50deg
  [../]
  [./mc]
    type = TensorMechanicsPlasticMohrCoulombMulti
    cohesion = mc_coh
    friction_angle = mc_phi
    dilation_angle = mc_psi
    use_custom_returnMap = true
    yield_function_tolerance = 1E-5
    internal_constraint_tolerance = 1E-9
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '0 1E7'
  [../]
  [./strain]
    type = ComputeFiniteStrain
    block = 0
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    block = 0
    ep_plastic_tolerance = 1E-12
    plastic_models = mc
  [../]
[]


[Executioner]
  end_time = 5
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = planar_hard5
  exodus = false
  [./csv]
    type = CSV
    hide = 'f0 f1 f2 f3 f4 f5 s_xy s_xz s_yz Coh_analytic Coh_moose'
    execute_on = 'timestep_end'
  [../]
[]
