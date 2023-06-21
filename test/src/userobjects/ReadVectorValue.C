//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReadVectorValue.h"
#include <sstream>

registerMooseObject("MooseTestApp", ReadVectorValue);

InputParameters
ReadVectorValue::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<std::vector<RealVectorValue>>("vector_realvv",
                                                        "A vector of VectorValues.");
  return params;
}

ReadVectorValue::ReadVectorValue(const InputParameters & params)
  : GeneralUserObject(params),
    _vector_realvv(getParam<std::vector<RealVectorValue>>("vector_realvv"))
{
  std::vector<RealVectorValue> gold_vv;
  gold_vv.push_back(RealVectorValue(0.1, 0.2, 0.3));
  gold_vv.push_back(RealVectorValue(0.4, 0.5, 0.6));

  if (_vector_realvv.size() != 2)
    mooseError("Error reading vector of RealVectorValues.");
  else
    for (auto vector_i : index_range(_vector_realvv))
      if (_vector_realvv[vector_i] != gold_vv[vector_i])
        mooseError("Error reading vector of RealVectorValues.");
}
