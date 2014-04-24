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

#include "OutputTestMaterial.h"

template<>
InputParameters validParams<OutputTestMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::string>("real_property_name", "real_property", "The name of the scalar real property");
  params.addParam<std::string>("vector_property_name", "vector_property", "The name of the vector real property");
  params.addParam<std::string>("tensor_property_name", "tensor_property", "The name of the tensor real property");
  params.addParam<Real>("real_factor", 0, "Add this factor to all of the scalar real material property");
  return params;
}

OutputTestMaterial::OutputTestMaterial(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _real_property(declareProperty<Real>(getParam<std::string>("real_property_name"))),
    _vector_property(declareProperty<RealVectorValue>(getParam<std::string>("vector_property_name"))),
    _tensor_property(declareProperty<RealTensorValue>(getParam<std::string>("tensor_property_name"))),
    _factor(getParam<Real>("real_factor"))
{
}

OutputTestMaterial::~OutputTestMaterial()
{
}

void
OutputTestMaterial::computeQpProperties()
{
  Point p = _q_point[_qp];

  _real_property[_qp] = p(0)*p(1) + _factor;

  RealVectorValue vec(p(0), p(1));
  _vector_property[_qp] = vec;

  RealTensorValue tensor(p(0)*p(0), p(0)*p(1), 0, -p(1)*p(0), p(1)*p(1));
  _tensor_property[_qp] = tensor;

}
