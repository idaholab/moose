# Frequency Response function for cantilever beam:
# Analytic results: 509Hz and 763Hz
# Simulation results with coarse mesh: 600Hz and 800Hz

[Mesh]
   type = GeneratedMesh
   elem_type = HEX8
   dim = 3
   xmin=0
   xmax=1
   nx=10
   ymin=0
   ymax=0.1
   ny = 1
   zmin=0
   zmax=0.15
   nz = 2
[]

[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y disp_z'
[]

[Problem]
 type = ReferenceResidualProblem
 reference_vector = 'ref'
 extra_tag_vectors = 'ref'
 group_variables = 'disp_x disp_y disp_z'
[]

[Modules]
  [TensorMechanics]
    [Master]
      [all]
        strain = SMALL
        add_variables = true
        new_system = true
        formulation = TOTAL
      []
    []
  []
[]

[Kernels]
    #reaction terms
    [reaction_realx]
        type = Reaction
        variable = disp_x
        rate = 0# filled by controller
        extra_vector_tags = 'ref'
    []
    [reaction_realy]
        type = Reaction
        variable = disp_y
        rate = 0# filled by controller
        extra_vector_tags = 'ref'
    []
    [reaction_realz]
        type = Reaction
        variable = disp_z
        rate = 0# filled by controller
        extra_vector_tags = 'ref'
    []
[]

[AuxVariables]
  [disp_mag]
  []
[]

[AuxKernels]
  [disp_mag]
    type = ParsedAux
    variable = disp_mag
    coupled_variables = 'disp_x disp_y disp_z'
    expression = 'sqrt(disp_x^2+disp_y^2+disp_z^2)'
  []
[]

[BCs]
#Left
[disp_x_left]
  type = DirichletBC
  variable = disp_x
  boundary = 'left'
  value = 0.0
[]
[disp_y_left]
  type = DirichletBC
  variable = disp_y
  boundary = 'left'
  value = 0.0
[]
[disp_z_left]
  type = DirichletBC
  variable = disp_z
  boundary = 'left'
  value = 0.0
[]
#Right
[BC_right_yreal]
    type = NeumannBC
    variable = disp_y
    boundary = 'right'
    value = 1000
[]
[BC_right_zreal]
    type = NeumannBC
    variable = disp_z
    boundary = 'right'
    value = 1000
[]
[]

[Materials]
  [elastic_tensor_Al]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 68e9
    poissons_ratio = 0.36
  []
  [compute_stress]
    type = ComputeLagrangianLinearElasticStress
  []
[]

[Postprocessors]
  [dispMag]
    type = NodalExtremeValue
    value_type = max
    variable = disp_mag
  []
[]

[Functions]
  [./freq2]
    type = ParsedFunction
    symbol_names = density
    symbol_values = 2.7e3 #Al kg/m3
    expression = '-t*t*density'
  [../]
[]

[Controls]
  [./func_control]
    type = RealFunctionControl
    parameter = 'Kernels/*/rate'
    function = 'freq2'
    execute_on = 'initial timestep_begin'
  [../]
[]

[Executioner]
  type = Transient
  solve_type=LINEAR
  petsc_options_iname = ' -pc_type'
  petsc_options_value = 'lu'
  start_time = 300  #starting frequency
  end_time =  1200  #ending frequency
  nl_abs_tol = 1e-6
  [TimeStepper]
    type = ConstantDT
    dt = 50  #frequency stepsize
  []
[]

[Outputs]
  csv=true
  exodus=false
  console = false
[]
