/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MacroElastic.h"

#include "SymmElasticityTensor.h"

template <>
InputParameters
validParams<MacroElastic>()
{
  InputParameters params = validParams<Elastic>();
  params.addRequiredParam<PostprocessorName>("C1111",
                                             "The postprocessor associated with the C1111 value");
  params.addRequiredParam<PostprocessorName>("C1122",
                                             "The postprocessor associated with the C1122 value");
  params.addRequiredParam<PostprocessorName>("C1133",
                                             "The postprocessor associated with the C1133 value");
  params.addRequiredParam<PostprocessorName>("C2222",
                                             "The postprocessor associated with the C2222 value");
  params.addRequiredParam<PostprocessorName>("C2233",
                                             "The postprocessor associated with the C2233 value");
  params.addRequiredParam<PostprocessorName>("C3333",
                                             "The postprocessor associated with the C3333 value");
  params.addRequiredParam<PostprocessorName>("C1212",
                                             "The postprocessor associated with the C1212 value");
  params.addRequiredParam<PostprocessorName>("C2323",
                                             "The postprocessor associated with the C2323 value");
  params.addRequiredParam<PostprocessorName>("C3131",
                                             "The postprocessor associated with the C3131 value");
  return params;
}

MacroElastic::MacroElastic(const InputParameters & parameters)
  : Elastic(parameters),
    _C1111(getPostprocessorValue("C1111")),
    _C1122(getPostprocessorValue("C1122")),
    _C1133(getPostprocessorValue("C1133")),
    _C2222(getPostprocessorValue("C2222")),
    _C2233(getPostprocessorValue("C2233")),
    _C3333(getPostprocessorValue("C3333")),
    _C1212(getPostprocessorValue("C1212")),
    _C2323(getPostprocessorValue("C2323")),
    _C3131(getPostprocessorValue("C3131"))
{
}

////////////////////////////////////////////////////////////////////////

MacroElastic::~MacroElastic() {}

////////////////////////////////////////////////////////////////////////

bool
MacroElastic::updateElasticityTensor(SymmElasticityTensor & tensor)
{
  std::vector<Real> v(9);
  v[0] = _C1111;
  v[1] = _C1122;
  v[2] = _C1133;
  v[3] = _C2222;
  v[4] = _C2233;
  v[5] = _C3333;
  v[6] = _C1212;
  v[7] = _C2323;
  v[8] = _C3131;

  tensor.fillFromInputVector(v, false);

  return true;
}

void
MacroElastic::createElasticityTensor()
{
  elasticityTensor(new SymmElasticityTensor(false));
}
