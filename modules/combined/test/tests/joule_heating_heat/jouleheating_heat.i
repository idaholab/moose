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
  [./elec]
  [../]
[]

[AuxVariables]
  [./elec_conduct]
      order = FIRST
      family = MONOMIAL
  [../]
  [./e_heatsrc]
      order = FIRST
      family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = HeatConduction
    variable = T
  [../]
  [./HeatTdot]
    type = HeatConductionTimeDerivative
    variable = T
  [../]
  [./HeatSrc]
    type = JouleHeatingSource
    variable = T
    elec = elec
  [../]
  [./electric]
    type = HeatConduction
    variable = elec
    diffusion_coefficient = electrical_conductivity
  [../]
[]

[AuxKernels]
  [./elec_conduct]
    type = MaterialRealAux
    variable = elec_conduct
    property = electrical_conductivity
    execute_on = timestep_end
  [../]
  [./elec_heatsrc]
    type = JouleHeatingHeatGeneratedAux
    variable = e_heatsrc
    elec = elec
#    electrical_conductivity = sigma
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./inlet]
    type = DirichletBC
    variable = T
    boundary = left
    value = 400 # K
  [../]
  [./outlet]
    type = DirichletBC
    variable = T
    boundary = right
    value = 600 # K
  [../]
  [./elec_left]
    type = DirichletBC
    variable = elec
    boundary = left
    value = 0
  [../]
  [./elec_right]
    type = DirichletBC
    variable = elec
    boundary = right
    value = 10
  [../]
[]

[Materials]
  [./k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '10' # in W/mK
  [../]
  [./cp]
    type = GenericConstantMaterial
    prop_names = 'specific_heat'
    prop_values = '390'   # in J/(KgK)
  [../]
  [./rho]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '9000'   # in Kg/m^3
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
    variable = 'T elec_conduct elec e_heatsrc'
    start_point = '0 0. 0'
    end_point = '1.0 0. 0'
    num_points = 21
    sort_by = id
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  nl_rel_tol = 1e-4
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  dt = 0.5
  end_time = 5
[]

[Outputs]
  exodus = true
  csv = true
[]

