/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "TestStatefulTensor.h"
#include "RankFourTensor.h"

template<>
InputParameters validParams<TestStatefulTensor>()
{
  InputParameters params = validParams<Material>();
  return params;
}

TestStatefulTensor::TestStatefulTensor(const std::string & name, InputParameters parameters) : Material(name, parameters),
   _tensor(declareProperty<RankFourTensor>("tensor")),
   _tensor_old(declareProperty<RankFourTensor>("tensor"))
{}

void
TestStatefulTensor::initQpStatefulProperties()
{
  _tensor[_qp].fillFromInputVector(std::vector<Real>(81,1), RankFourTensor::general);
}

void
TestStatefulTensor::computeQpProperties()
{
  _tensor[_qp] = _tensor_old[_qp] * 2;
}
