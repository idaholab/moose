# Test thermophysical property calculations using the air external submodule fluid properties

[Mesh]
    type = GeneratedMesh
    dim = 2
    # This test uses ElementalVariableValue postprocessors on specific
    # elements, so element numbering needs to stay unchanged
    allow_renumbering = false
  []

  [Variables]
    [dummy]
    []
  []

  [AuxVariables]
    [pressure]
      initial_condition = 2e6
      family = MONOMIAL
      order = CONSTANT
    []
    [temperature]
      initial_condition = 350
      family = MONOMIAL
      order = CONSTANT
    []
    [rho]
      family = MONOMIAL
      order = CONSTANT
    []
    [mu]
      family = MONOMIAL
      order = CONSTANT
    []
    [e]
      family = MONOMIAL
      order = CONSTANT
    []
    [h]
      family = MONOMIAL
      order = CONSTANT
    []
    [s]
      family = MONOMIAL
      order = CONSTANT
    []
    [cv]
      family = MONOMIAL
      order = CONSTANT
    []
    [cp]
      family = MONOMIAL
      order = CONSTANT
    []
    [c]
      family = MONOMIAL
      order = CONSTANT
    []
  []

  [AuxKernels]
    [rho]
      type = MaterialRealAux
      variable = rho
      property = density
    []
    [my]
      type = MaterialRealAux
      variable = mu
      property = viscosity
    []
    [internal_energy]
      type = MaterialRealAux
      variable = e
      property = e
    []
    [enthalpy]
      type = MaterialRealAux
      variable = h
      property = h
    []
    [entropy]
      type = MaterialRealAux
      variable = s
      property = s
    []
    [cv]
      type = MaterialRealAux
      variable = cv
      property = cv
    []
    [cp]
      type = MaterialRealAux
      variable = cp
      property = cp
    []
    [c]
      type = MaterialRealAux
      variable = c
      property = c
    []
  []


[Materials]
    [fp_mat]
      type = FluidPropertiesMaterialPT
      pressure = pressure
      temperature = temperature
      fp = fp
    []
  []

  [Kernels]
    [diff]
      type = Diffusion
      variable = dummy
    []
  []

  [Executioner]
    type = Steady
    solve_type = NEWTON
  []

  [Problem]
    solve = false
  []

  [Postprocessors]
    [rho]
      type = ElementalVariableValue
      elementid = 0
      variable = rho
    []
    [mu]
      type = ElementalVariableValue
      elementid = 0
      variable = mu
    []
    [e]
      type = ElementalVariableValue
      elementid = 0
      variable = e
    []
    [h]
      type = ElementalVariableValue
      elementid = 0
      variable = h
    []
    [s]
      type = ElementalVariableValue
      elementid = 0
      variable = s
    []
    [cv]
      type = ElementalVariableValue
      elementid = 0
      variable = cv
    []
    [cp]
      type = ElementalVariableValue
      elementid = 0
      variable = cp
    []
    [c]
      type = ElementalVariableValue
      elementid = 0
      variable = c
    []
  []

  [Outputs]
    csv = true
    execute_on = 'TIMESTEP_END'
  []
