//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantVectorPostprocessor.h"

registerMooseObject("MooseApp", ConstantVectorPostprocessor);

InputParameters
ConstantVectorPostprocessor::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription(
      "Populate constant VectorPostprocessorValue directly from input file.");
  params.addParam<std::vector<std::string>>("vector_names",
                                            std::vector<std::string>(1, "value"),
                                            "Names of the column vectors in this object");
  params.addRequiredParam<std::vector<std::vector<Real>>>(
      "value",
      "Vector values this object will have. Leading dimension must be equal to leading dimension "
      "of vector_names parameter.");

  return params;
}

ConstantVectorPostprocessor::ConstantVectorPostprocessor(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters)
{
  std::vector<std::string> names = getParam<std::vector<std::string>>("vector_names");
  unsigned int nvec = names.size();

  _value.resize(nvec);
  for (unsigned int j = 0; j < nvec; ++j)
    _value[j] = &declareVector(names[j]);

  std::vector<std::vector<Real>> v = getParam<std::vector<std::vector<Real>>>("value");
  if (v.size() != nvec)
    paramError("value",
               "Leading dimension must be equal to leading dimension of vector_names parameter.");

  if (processor_id() == 0)
  {
    for (unsigned int j = 0; j < nvec; ++j)
    {
      unsigned int ne = v[j].size();
      _value[j]->resize(ne);
      for (unsigned int l = 0; l < ne; ++l)
        (*_value[j])[l] = v[j][l];
    }
  }
}

void
ConstantVectorPostprocessor::initialize()
{
}

void
ConstantVectorPostprocessor::execute()
{
}
