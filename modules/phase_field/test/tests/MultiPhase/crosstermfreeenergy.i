[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  nz = 0
  xmin = -8
  xmax = 8
  ymin = -8
  ymax = 8
  elem_type = QUAD4
[]

[AuxVariables]
  [./local_energy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./cross_energy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./local_free_energy]
    type = TotalFreeEnergy
    f_name = F0
    variable = local_energy
    additional_free_energy = cross_energy
  [../]
  [./cross_terms]
    type = CrossTermGradientFreeEnergy
    variable = cross_energy
    interfacial_vars = 'eta1 eta2 eta3'
    kappa_names = 'kappa11 kappa12 kappa13
                   kappa21 kappa22 kappa23
                   kappa31 kappa32 kappa33'
  [../]
[]

[Variables]
  [./eta1]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 0.0
      y1 = 5.0
      radius = 5.0
      invalue = 1.0
      outvalue = 0.0
      int_width = 10.0
    [../]
  [../]
  [./eta2]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = -4.0
      y1 = -2.0
      radius = 5.0
      invalue = 1.0
      outvalue = 0.0
      int_width = 10.0
    [../]
  [../]
  [./eta3]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 4.0
      y1 = -2.0
      radius = 5.0
      invalue = 1.0
      outvalue = 0.0
      int_width = 10.0
    [../]
  [../]
[]

[Kernels]
  [./dummy_diff1]
    type = Diffusion
    variable = eta1
  [../]
  [./dummy_time1]
    type = TimeDerivative
    variable = eta1
  [../]
  [./dummy_diff2]
    type = Diffusion
    variable = eta2
  [../]
  [./dummy_time2]
    type = TimeDerivative
    variable = eta2
  [../]
  [./dummy_diff3]
    type = Diffusion
    variable = eta3
  [../]
  [./dummy_tim3]
    type = TimeDerivative
    variable = eta3
  [../]
[]

[Materials]
  [./consts]
    type = GenericConstantMaterial
    prop_names  = 'F0   kappa11 kappa12 kappa13 kappa21 kappa22 kappa23 kappa31 kappa32 kappa33'
    prop_values = '0    11      12      13      12      22      23      13      23      33     '
  [../]

[]

[Executioner]
  type = Transient
  dt = 0.001
  num_steps = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  [./out]
    type = Exodus
    hide = 'eta1 eta2 eta3 local_energy'
  [../]
[]
