#
# This tests the WeightedVariableAverage postprocessor, which averages a variable field
# with weights applied from a material property. This can be used to obtain average
# concentrations in different phases (based on the total physical concentration variable).
#
#

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
[]

[Variables]
  [c]
    [./InitialCondition]
      type = FunctionIC
      function = x*0.5
    []
   []
   [eta]
     [./InitialCondition]
       type = FunctionIC
       function = x
     []
   []
 []

 [Materials]
   [h]
     type = ParsedMaterial
     coupled_variables = eta
     property_name = h
     expression = 'if(eta>0.5,1,0)'
   []
 []

 [Postprocessors]
   [c1]
     type = WeightedVariableAverage
     v = c
     weight = h
     execute_on = INITIAL
   []
 []

 [Problem]
   solve = false
 []

 [Executioner]
   type = Steady
 []

 [Outputs]
   csv = true
 []
