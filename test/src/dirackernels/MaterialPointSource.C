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

#include "MaterialPointSource.h"

template <>
InputParameters
validParams<MaterialPointSource>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<Point>("point", "The x,y,z coordinates of the point");
  params.addParam<MaterialPropertyName>(
      "material_prop", "matp", "the name of the material property for the coefficient");

  MooseEnum prop_state("current old older", "current");
  params.addParam<MooseEnum>(
      "prop_state", prop_state, "Declares which property state we should retrieve");

  params.declareControllable("point");
  return params;
}

MaterialPointSource::MaterialPointSource(const InputParameters & parameters)
  : DiracKernel(parameters), _p(getParam<Point>("point"))
{
  auto mat_prop_name = getParam<MaterialPropertyName>("material_prop");
  auto prop_state = getParam<MooseEnum>("prop_state");

  if (prop_state == "current")
    _value = &getMaterialProperty<Real>(mat_prop_name);
  else if (prop_state == "old")
    _value = &getMaterialPropertyOld<Real>(mat_prop_name);
  else if (prop_state == "older")
    _value = &getMaterialPropertyOlder<Real>(mat_prop_name);
}

void
MaterialPointSource::addPoints()
{
  addPoint(_p);
}

Real
MaterialPointSource::computeQpResidual()
{
  // These values should match... this shows the problem
  // Moose::out << "_value[_qp]=" << _value[_qp] << std::endl;
  // Moose::out << "_q_point[_qp](0)=" << _q_point[_qp](0) << std::endl;

  // This is negative because it's a forcing function that has been
  // brought over to the left side.
  return -_test[_i][_qp] * (*_value)[_qp];
}
