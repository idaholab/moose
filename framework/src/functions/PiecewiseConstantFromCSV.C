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
  params.addRequiredParam<unsigned int>("column_number",
                                        "The column number for the desired data in the CSV");

  // This parameter is added for optimization when doing nearest neighbor interpolation
  // but it's also safer to have that parameter be close to the use case, and not only in the UO
  params.addRequiredParam<MooseEnum>("read_type",
                                     MooseEnum("element voronoi block"),
                                     "Organization of data in the CSV file: "
                                     "element:by element "
                                     "voronoi:nearest neighbor / voronoi tesselation structure "
                                     "block:by mesh block");

  params.addClassDescription("Uses data read from CSV to assign values");
  return params;
}

PiecewiseConstantFromCSV::PiecewiseConstantFromCSV(const InputParameters & parameters)
  : Function(parameters),
    _read_prop_user_object(nullptr),
    _column_number(getParam<unsigned int>("column_number")),
    _read_type(getParam<MooseEnum>("read_type").getEnum<ReadTypeEnum>()),
    _point_locator(_ti_feproblem.mesh().getPointLocator())
{
  if (_column_number < _ti_feproblem.mesh().dimension() && _read_type == ReadTypeEnum::VORONOI)
    mooseWarning(
        "The column requested in the function is likely to just be containing point coordinates");
}

void
PiecewiseConstantFromCSV::initialSetup()
{
  // Get a pointer to the ElementPropertyReadFile. A pointer is used because the UserObject is not
  // available during the construction of the function
  _read_prop_user_object = &getUserObject<ElementPropertyReadFile>("read_prop_user_object");

  if (_read_type != _read_prop_user_object->getReadType())
    paramError("read_type",
               "The ElementPropertyReadFile UO should have the same read_type parameter.");
}

Real
PiecewiseConstantFromCSV::value(Real, const Point & p) const
{
  if (_read_type != ReadTypeEnum::VORONOI)
  {
    // This is somewhat inefficient, but it allows us to retrieve the data in the
    // CSV by element or by block.
    const auto current_elem = (*_point_locator)(p);

    // A point may be on the boundary of some elements
    // auto elem_id = current_elem ? current_elem->id() : DofObject::invalid_id;
    // _communicator.min(elem_id);

    if (elem_id == DofObject::invalid_id)
      mooseError("No element located at ", p, " in PointValue Postprocessor named: ", name());

    return _read_prop_user_object->getData(current_elem, _column_number);
  }
  else
    // No need to search for the element if we're just looking at nearest neighbors
    return _read_prop_user_object->getVoronoiData(p, _column_number);
}
