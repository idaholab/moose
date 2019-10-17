#include "EBCoupledVarTest.h"

registerMooseObject("PhaseFieldApp", EBCoupledVarTest);

template <>
InputParameters
validParams<EBCoupledVarTest>()
{
  InputParameters params = validParams<DerivativeParsedMaterialHelper>();
  params.addClassDescription("Test to see if vector of coupled variables works with ExpressionBuilder");
  params.addRequiredCoupledVarWithAutoBuild(
      "v","var_name_base","op_num","Array of coupled variables");

  return params;
}

EBCoupledVarTest::EBCoupledVarTest(const InputParameters & parameters)
  : DerivativeParsedMaterialHelper(parameters),
    _op_num(coupledComponents("v")),
    _vals(_op_num)
{
  EBTerm newest("v");
  std::string new_name;
  for(unsigned int i = 0; i < _op_num; ++i)
  {
    new_name = "v" + std::to_string(i);
    EBTerm new_term(new_name.c_str());
    _vals[i] = new_term;
  }
  EBFunction tester;
  tester(_vals) = _vals[0] + 2 * _vals[1];

  functionParse(tester);
}
