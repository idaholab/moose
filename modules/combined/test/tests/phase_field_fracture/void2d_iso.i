[Mesh]
  type = FileMesh
  file = void2d_mesh.xda
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Modules]
  [./TensorMechanics]
    [./Master]
      [./All]
        add_variables = true
        strain = SMALL
        additional_generate_output = stress_yy
      [../]
    [../]
  [../]
  [./PhaseField]
    [./Nonconserved]
      [./c]
        free_energy = F
        mobility = L
        kappa = kappa_op
      [../]
    [../]
  [../]
[]

[Functions]
  [./tfunc]
    type = ParsedFunction
    expression = t
  [../]
  [./void_prop_func]
    type = ParsedFunction
    expression = 'rad:=0.2;m:=50;r:=sqrt(x^2+y^2);1-exp(-(r/rad)^m)+1e-8'
  [../]
  [./gb_prop_func]
    type = ParsedFunction
    expression = 'rad:=0.2;thk:=0.05;m:=50;sgnx:=1-exp(-(x/rad)^m);v:=sgnx*exp(-(y/thk)^m);0.005*(1-v)+0.001*v'
  [../]
[]

[Kernels]
  [./solid_x]
    type = PhaseFieldFractureMechanicsOffDiag
    variable = disp_x
    component = 0
    c = c
  [../]
  [./solid_y]
    type = PhaseFieldFractureMechanicsOffDiag
    variable = disp_y
    component = 1
    c = c
  [../]
[]

[BCs]
  [./ydisp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = tfunc
  [../]
  [./yfix]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./xfix]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
[]

[Materials]
  [./pfbulkmat]
    type = GenericConstantMaterial
    prop_names = 'l visco'
    prop_values = '0.01 0.1'
  [../]
  [./pfgc]
    type = GenericFunctionMaterial
    prop_names = 'gc_prop'
    prop_values = 'gb_prop_func'
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '120.0 80.0'
    fill_method = symmetric_isotropic
    elasticity_tensor_prefactor = void_prop_func
  [../]
  [./define_mobility]
    type = ParsedMaterial
    material_property_names = 'gc_prop visco'
    property_name = L
    expression = '1.0/(gc_prop * visco)'
  [../]
  [./define_kappa]
    type = ParsedMaterial
    material_property_names = 'gc_prop l'
    property_name = kappa_op
    expression = 'gc_prop * l'
  [../]
  [./damage_stress]
    type = ComputeLinearElasticPFFractureStress
    c = c
    E_name = 'elastic_energy'
    D_name = 'degradation'
    F_name = 'fracture_energy'
    decomposition_type = strain_spectral
  [../]
  [./degradation]
    type = DerivativeParsedMaterial
    property_name = degradation
    coupled_variables = 'c'
    expression = '(1.0-c)^2*(1.0 - eta) + eta'
    constant_names       = 'eta'
    constant_expressions = '0.0'
    derivative_order = 2
  [../]
  [./fracture_energy]
    type = DerivativeParsedMaterial
    property_name = fracture_energy
    coupled_variables = 'c'
    material_property_names = 'gc_prop l'
    expression = 'c^2 * gc_prop / 2 / l'
    derivative_order = 2
  [../]
  [./fracture_driving_energy]
    type = DerivativeSumMaterial
    coupled_variables = c
    sum_materials = 'elastic_energy fracture_energy'
    derivative_order = 2
    property_name = F
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
  solve_type = PJFNK

  petsc_options_iname = '-pc_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm      lu           1'

  nl_rel_tol = 1e-9
  nl_max_its = 10
  l_tol = 1e-4
  l_max_its = 40

  dt = 1e-4
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
