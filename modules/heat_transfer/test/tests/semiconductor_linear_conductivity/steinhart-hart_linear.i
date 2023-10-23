[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 1.0
  ymax = 1.0
[]

[Variables]
  [./T]
      initial_condition = 400.0   # unit in Kelvin only!!
  [../]
[]

[AuxVariables]
  [./elec_conduct]
      order = FIRST
      family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = HeatConduction
    variable = T
  [../]
[]

[AuxKernels]
  [./elec_conduct]
    type = MaterialRealAux
    variable = elec_conduct
    property = electrical_conductivity
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./inlet]
    type = DirichletBC
    variable = T
    boundary = left
    value = 1000 # K
  [../]
  [./outlet]
    type = DirichletBC
    variable = T
    boundary = right
    value = 400 # K
  [../]
[]

[Materials]
  [./k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '10' # in W/mK
  [../]
  [./sigma]
    type = SemiconductorLinearConductivity
    temp = T
    sh_coeff_A = 0.002
    sh_coeff_B = 0.001
  [../]
[]

[VectorPostprocessors]
  [./line_sample]
    type = LineValueSampler
    warn_discontinuous_face_values = false
    variable = 'T elec_conduct'
    start_point = '0 0. 0'
    end_point = '1.0 0. 0'
    num_points = 11
    sort_by = id
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_rel_tol = 1e-12
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  execute_on = 'initial timestep_end'
[]
