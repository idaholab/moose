# This test is to ensure that the smeared cracking model correctly handles finite
# rotation of cracked elements.

# This consists of a single element that is first  subjected to tensile loading
# in the y-direction via a prescribed displacement. This loading is sufficiently
# high to crack the material in that direction, but not completely unload. The
# prescribed displacement is then reversed so that the element is returned to its
# original configuration.

# In the next phase of the analysis, this element is then rotated 90 degrees by
# prescribing the displacement of the bottom of the element. The prescribed
# displacement BC used to crack the element in the first phase is deactivated.

# Once the element is fully rotated, a new BC is activated on what was originally
# the top surface (but is now the surface on the right hand side) to pull in
# the x-direction.

# If everything is working correctly, the model should re-load on the original
# crack (which should be rotated along with the elemnent) up to the peak stress
# in the first phase of the analysis, and then continue the unloading process
# as the crack strains continue to increase. Throughout this analysis, there should
# only be a single crack, as manifested in the crack_flags variables.

[Mesh]
  file = cracking_test.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
  [../]
[]

[AuxVariables]
  [./crack_flags1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./crack_flags2]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./crack_flags3]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./crack_flags1]
    type = MaterialRealVectorValueAux
    property = crack_flags
    variable = crack_flags1
    component = 0
  [../]
  [./crack_flags2]
    type = MaterialRealVectorValueAux
    property = crack_flags
    variable = crack_flags2
    component = 1
  [../]
  [./crack_flags3]
    type = MaterialRealVectorValueAux
    property = crack_flags
    variable = crack_flags3
    component = 2
  [../]
[]

[BCs]
  [./x_pin]
    type = DirichletBC
    variable = disp_x
    boundary = '15 16'
    value = 0.0
  [../]
  [./y_pin]
    type = DirichletBC
    variable = disp_y
    boundary = '15 16'
    value = 0.0
  [../]
  [./z_all]
    type = DirichletBC
    variable = disp_z
    boundary = '11 12 13 14 15 16 17 18'
    value = 0.0
  [../]
  [./x_lb]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = '11 12'
    function = 'if(t<10,0,if(t>=100,1,1-cos((t-10)*pi/180)))'
  [../]
  [./y_lb]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = '11 12'
    function = 'if(t<10,0,if(t>=100,1,sin((t-10)*pi/180)))'
  [../]
  [./x_lt]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = '13 14'
    function = '2+(t-100)*0.01'
  [../]
  [./x_rt]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = '17 18'
    function = '1+(t-100)*0.01'
  [../]
  [./top_pull]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = '13 14 17 18'
    function = 'if(t<5,t*0.01,0.05-(t-5)*0.01)'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 100.e9
    poissons_ratio = 0.
  [../]
  [./cracking_stress]
    type = ComputeSmearedCrackingStress
    shear_retention_factor = 0.1
    cracking_stress = 3.e9
    softening_models = exponential_softening
  [../]
  [./exponential_softening]
    type = ExponentialSoftening
  [../]
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-ksp_gmres_restart -pc_type'
  petsc_options_value = '101                lu'
  line_search = 'none'

  l_max_its = 100
  l_tol = 1e-5

  nl_max_its = 100
  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-12

  start_time = 0
  end_time = 110
  dt = 1
[]

[Controls]
  [./p1]
    type = TimePeriod
    start_time = 0.0
    end_time = 10.0
    disable_objects = 'BCs/x_lt BCs/x_rt'
    enable_objects = 'BCs/top_pull'
    reverse_on_false = false
    execute_on = 'initial timestep_begin'
  [../]
  [./p2]
    type = TimePeriod
    start_time = 10.0
    end_time = 101.0
    disable_objects = 'BCs/x_lt BCs/x_rt BCs/top_pull'
    reverse_on_false = false
    execute_on = 'initial timestep_begin'
  [../]
  [./p3]
    type = TimePeriod
    start_time = 101.0
    end_time = 110.0
    enable_objects = 'BCs/x_lt BCs/x_rt'
    disable_objects = 'BCs/top_pull'
    reverse_on_false = false
    execute_on = 'initial timestep_begin'
  [../]
[]

[Outputs]
  exodus = true
[]
