# A 100m x 10m "slab" of height 100m is subjected to cyclic pressure at its top
# Assumptions:
# the boundaries are impermeable, except the top boundary
# only vertical displacement is allowed
# the atmospheric pressure sets the total stress at the top of the model
# at the slab left-hand side there is a borehole that taps into the base of the slab.
[Mesh]
  [the_mesh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    ny = 1
    nz = 10
    xmin = 0
    xmax = 100
    ymin = -5
    ymax = 5
    zmin = -100
    zmax = 0
  []
  [bh_back]
    type = ExtraNodesetGenerator
    coord = '0 -5 -100'
    input = the_mesh
    new_boundary = 11
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  PorousFlowDictator = dictator
  block = 0
  biot_coefficient = 0.6
  multiply_by_density = false
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [porepressure]
    scaling = 1E11
  []
[]

[ICs]
  [porepressure]
    type = FunctionIC
    variable = porepressure
    function = '-10000*z'  # this is only approximately correct
  []
[]

[Functions]
  [ini_stress_zz]
    type = ParsedFunction
    expression = '(25000 - 0.6*10000)*z' # remember this is effective stress
  []
  [cyclic_porepressure]
    type = ParsedFunction
    expression = 'if(t>0,5000 * sin(2 * pi * t / 3600.0 / 24.0),0)'
  []
  [cyclic_porepressure_at_depth]
    type = ParsedFunction
    expression = '-10000*z + if(t>0,5000 * sin(2 * pi * t / 3600.0 / 24.0),0)'
  []
  [neg_cyclic_porepressure]
    type = ParsedFunction
    expression = '-if(t>0,5000 * sin(2 * pi * t / 3600.0 / 24.0),0)'
  []
[]

[BCs]
  # zmin is called 'back'
  # zmax is called 'front'
  # ymin is called 'bottom'
  # ymax is called 'top'
  # xmin is called 'left'
  # xmax is called 'right'
  [no_x_disp]
    type = DirichletBC
    variable = disp_x
    value = 0
    boundary = 'bottom top' # because of 1-element meshing, this fixes u_x=0 everywhere
  []
  [no_y_disp]
    type = DirichletBC
    variable = disp_y
    value = 0
    boundary = 'bottom top' # because of 1-element meshing, this fixes u_y=0 everywhere
  []
  [no_z_disp_at_bottom]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = back
  []
  [pp]
    type = FunctionDirichletBC
    variable = porepressure
    function = cyclic_porepressure
    boundary = front
  []
  [pp_downhole]
    type = FunctionDirichletBC
    variable = porepressure
    function = cyclic_porepressure_at_depth
    boundary = 11
  []
  [total_stress_at_top]
    type = FunctionNeumannBC
    variable = disp_z
    function = neg_cyclic_porepressure
    boundary = front
  []
[]

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    thermal_expansion = 0.0
    bulk_modulus = 2E9
    viscosity = 1E-3
    density0 = 1000.0
  []
[]

[PorousFlowBasicTHM]
  coupling_type = HydroMechanical
  displacements = 'disp_x disp_y disp_z'
  porepressure = porepressure
  gravity = '0 0 -10'
  fp = the_simple_fluid
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    bulk_modulus = 10.0E9 # drained bulk modulus
    poissons_ratio = 0.25
  []
  [strain]
    type = ComputeSmallStrain
    eigenstrain_names = ini_stress
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '0 0 0  0 0 0  0 0 ini_stress_zz'
    eigenstrain_name = ini_stress
  []
  [porosity]
    type = PorousFlowPorosityConst # only the initial value of this is ever used
    porosity = 0.1
  []
  [biot_modulus]
    type = PorousFlowConstantBiotModulus
    solid_bulk_compliance = 1E-10
    fluid_bulk_modulus = 2E9
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-14 0 0   0 1E-14 0   0 0 1E-14'
  []
  [density]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 2500.0
  []
[]

[Postprocessors]
  [p0_0]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = porepressure
  []
  [p100_0]
    type = PointValue
    outputs = csv
    point = '100 0 0'
    variable = porepressure
  []
  [p0_100]
    type = PointValue
    outputs = csv
    point = '0 0 -100'
    variable = porepressure
  []
  [p100_100]
    type = PointValue
    outputs = csv
    point = '100 0 -100'
    variable = porepressure
  []
  [uz0]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = disp_z
  []
  [uz100]
    type = PointValue
    outputs = csv
    point = '100 0 0'
    variable = disp_z
  []
[]


[Preconditioning]
  [andy]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  start_time = -3600
  dt = 3600
  end_time = 172800
  nl_rel_tol = 1E-10
  nl_abs_tol = 1E-5
[]

[Outputs]
  print_linear_residuals = false
  csv = true
[]
