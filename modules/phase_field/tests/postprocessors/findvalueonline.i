[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 10
[]

[Variables]
  [./phi]
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 0
      y1 = 0
      radius = 3
      invalue = 1
      outvalue = 0
      int_width = 2
    [../]
  [../]
[]

[Kernels]
  [./ac]
    type = AllenCahn
    variable = phi
    f_name = F
    mob_name = 1
  [../]
  [./aci]
    type = ACInterface
    kappa_name = 0.2
    variable = phi
    mob_name = 1
  [../]
  [./dt]
    type = TimeDerivative
    variable = phi
  [../]
[]

[Materials]
  [./F]
    type = DerivativeParsedMaterial
    args = phi
    function = phi^2*(1-phi)^2-0.1*phi
  [../]
[]

[Postprocessors]
  [./pos]
    type = FindValueOnLine
    target = 0.5
    depth = 20
    v = phi
    start_point = '0 0 0'
    end_point = '10 10 0'
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 1
[]

[Outputs]
  csv = true
[]
