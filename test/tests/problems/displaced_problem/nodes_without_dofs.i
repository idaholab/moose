# The quadratic displacement disp_z = a x (x + z) is represented exactly by SECOND order Lagrange
# displacements. The displaced volume, 1 + a / 2, and the integral of z^4 over the displaced mesh,
# 0.30722669047619 for a = 0.1, are therefore exact only if the face nodes of the TET14 elements,
# which carry no SECOND order degrees of freedom, are displaced as well.
a = 0.1

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    elem_type = TET4
  []
  [complete]
    type = ElementOrderConversionGenerator
    input = gmg
    conversion_type = COMPLETE_ORDER
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[AuxVariables]
  [disp_x]
    order = SECOND
  []
  [disp_y]
    order = SECOND
  []
  [disp_z]
    order = SECOND
    [InitialCondition]
      type = FunctionIC
      function = '${a} * x * (x + z)'
    []
  []
[]

[Postprocessors]
  [volume]
    type = VolumePostprocessor
    use_displaced_mesh = true
  []
  [z4_integral]
    type = FunctionElementIntegral
    function = 'z^4'
    use_displaced_mesh = true
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
  # Integrates z^4 over the displaced mesh exactly: the integrand is a polynomial of degree ten
  [Quadrature]
    order = TENTH
  []
[]

[Outputs]
  csv = true
[]
