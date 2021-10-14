//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseConstantFromCSV.h"

registerMooseObject("MooseApp", PiecewiseConstantFromCSV);

InputParameters
PiecewiseConstantFromCSV::validParams()
{
  InputParameters params = Function::validParams();
  params.addRequiredParam<UserObjectName>("read_prop_user_object",
                                          "The ElementReadPropertyFile "
                                          "GeneralUserObject to read element "
                                          "specific property values from file");
  params.addRequiredParam<unsigned int>("column_number", "The column number for the desired data in the CSV");
  params.addClassDescription("Uses data read from CSV to assign values");
  return params;
}

PiecewiseConstantFromCSV::PiecewiseConstantFromCSV(const InputParameters & parameters)
  : Function(parameters),
    _read_prop_user_object(getUserObject<ElementPropertyReadFile>("read_prop_user_object")),
    _column_number(getParam<unsigned int>("column_number"))
{
}

Real
PiecewiseConstantFromCSV::value(Real t, const Point & p) const
{
  // This is somewhat inefficient, but it allows us to retrieve the data in the
  // CSV by element or by block. It's unnecessary for nearest neighbor
  const auto current_elem =

  _read_prop_user_object.getData(current_elem, _column_number);
}

Real
PiecewiseConstantFromCSV::timeDerivative(Real /*t*/, const Point & /*p*/) const
{
  return 0;
}
