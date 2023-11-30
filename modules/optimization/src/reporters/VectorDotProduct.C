//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorDotProduct.h"

registerMooseObject("OptimizationApp", VectorDotProduct);

InputParameters
VectorDotProduct::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Dot product between two vectors held in reporters.");

  params.addParam<std::string>(
      "name", "VectorDotProduct", "Name of reporter containing scalar result of dot product.");
  params.addParam<Real>("scale", 1, "Scale dot product");
  params.addRequiredParam<ReporterName>("vector_a", "First reporter vector in dot product.");
  params.addRequiredParam<ReporterName>("vector_b", "Second reporter vector in dot product.");
  // This reporters is for postprocessing optimization results and shold be exectuted at the end of
  // execution
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
  return params;
}

VectorDotProduct::VectorDotProduct(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _scale(getParam<Real>("scale")),
    _dot_product(declareValueByName<Real>(getParam<std::string>("name"), REPORTER_MODE_REPLICATED)),
    _vector_a(getReporterValueByName<std::vector<Real>>(getParam<ReporterName>("vector_a"),
                                                        REPORTER_MODE_REPLICATED)),
    _vector_b(getReporterValueByName<std::vector<Real>>(getParam<ReporterName>("vector_b"),
                                                        REPORTER_MODE_REPLICATED))
{
}

void
VectorDotProduct::finalize()
{
  std::size_t entries(_vector_a.size());
  if (entries != _vector_b.size())
    mooseError("Vectors in dot product must be same size.",
               "\nsize of 'vector_a = ",
               entries,
               "\nsize of 'vector_b = ",
               _vector_b.size());

  _dot_product = 0;
  for (std::size_t i = 0; i < entries; ++i)
    _dot_product += _vector_a[i] * _vector_b[i];
  _dot_product *= _scale;
}
