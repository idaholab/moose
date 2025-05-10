//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorCoupledOldAux.h"

registerMooseObject("MooseApp", VectorCoupledOldAux);

InputParameters
VectorCoupledOldAux::validParams()
{
  InputParameters params = VectorAuxKernel::validParams();
  params.addClassDescription("OldValueTestAux that return old value.");
  params.addRequiredCoupledVar("v", "This should be the same vector variable specified twice.");

  return params;
}

VectorCoupledOldAux::VectorCoupledOldAux(const InputParameters & parameters)
  : VectorAuxKernel(parameters), _vectors(coupledVectorValuesOld("v"))
{
  if (_vectors.size() != 2)
  {
    mooseError("v must have 2 entries not ", _vectors.size());
  }

  std::string name1 = getVectorVar("v", 0)->name();
  std::string name2 = getVectorVar("v", 1)->name();

  if (name1 != name2)
  {
    mooseError("v must contain the same entry specified twice.");
  }
}

RealVectorValue
VectorCoupledOldAux::computeValue()
{
  const VectorVariableValue & v1 = *_vectors[0];
  const VectorVariableValue & v2 = *_vectors[1];
  return 2. * v1[_qp] - v2[_qp];
}
