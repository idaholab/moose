[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
  []
[]


[Problem]
  solve = false
[]

[Variables]
  [a]
    [InitialCondition]
      type = FunctionIC
      function = x
    []
  []
  [b]
    [InitialCondition]
      type = FunctionIC
      function = y
    []
  []
  [c]
    [InitialCondition]
      type = FunctionIC
      function = z
    []
  []
[]

[Materials]
  [ctd]
    type = CTDCoupledVarTest
    x = a
    y = b
    z = c
    property_name = F
  []
  [parsed]
    type = DerivativeParsedMaterial
    coupled_variables = 'a b c'
    expression = 'a^3*b^4*c^5 + sin(a)*cos(b)/(c+0.1) + log(a+0.1)*sin(b)*cos(c)'
    property_name = G
  []

  [L2_difference]
    type = ParsedMaterial
    expression = '(f0-g0)^2+(f1-g1)^2+(f2-g2)^2+(f3-g3)^2+(f4-g4)^2+(f5-g5)^2+(f6-g6)^2+(f7-g7)^2+(f8-g8)^2+(f9-g9)^2+(f10-g10)^2+(f11-g11)^2+(f12-g12)^2+(f13-g13)^2+(f14-g14)^2+(f15-g15)^2+(f16-g16)^2+(f17-g17)^2+(f18-g18)^2+(f19-g19)^2'
    material_property_names = 'f0:=F g0:=G f1:=dF/da g1:=dG/da f2:=dF/db g2:=dG/db f3:=dF/dc g3:=dG/dc f4:=d^2F/da^2 g4:=d^2G/da^2 f5:=d^2F/dadb g5:=d^2G/dadb f6:=d^2F/dadc g6:=d^2G/dadc f7:=d^2F/db^2 g7:=d^2G/db^2 f8:=d^2F/dbdc g8:=d^2G/dbdc f9:=d^2F/dc^2 g9:=d^2G/dc^2 f10:=d^3F/da^2db g10:=d^3G/da^2db f11:=d^3F/da^2dc g11:=d^3G/da^2dc f12:=d^3F/da^3 g12:=d^3G/da^3 f13:=d^3F/dadb^2 g13:=d^3G/dadb^2 f14:=d^3F/dadbdc g14:=d^3G/dadbdc f15:=d^3F/dadc^2 g15:=d^3G/dadc^2 f16:=d^3F/db^2dc g16:=d^3G/db^2dc f17:=d^3F/db^3 g17:=d^3G/db^3 f18:=d^3F/dbdc^2 g18:=d^3G/dbdc^2 f19:=d^3F/dc^3 g19:=d^3G/dc^3'
    property_name = L2
  []
[]

[Postprocessors]
  [L2_int]
    type = ElementIntegralMaterialProperty
    mat_prop = L2
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
