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
                                          "The PropertyReadFile "
                                          "GeneralUserObject to read element "
                                          "specific property values from file");
  params.addRequiredParam<unsigned int>(
      "column_number", "The column number (0-indexing) for the desired data in the CSV");

  // This parameter is added for optimization when doing nearest neighbor interpolation
  // but it's also safer to have that parameter be close to the use case, and not only in the UO
  params.addRequiredParam<MooseEnum>("read_type",
                                     MooseEnum("element voronoi block node"),
                                     "Organization of data in the CSV file: "
                                     "element:by element "
                                     "node: by node "
                                     "voronoi:nearest neighbor / voronoi tesselation structure "
                                     "block:by mesh block");
  // We need one ghost layer in case we need to retrieve the id or subdomain id from an
  // element that is on the other side of a domain boundary
  params.set<unsigned short>("ghost_layers") = 1;

  params.addClassDescription("Uses data read from CSV to assign values");
  return params;
}

PiecewiseConstantFromCSV::PiecewiseConstantFromCSV(const InputParameters & parameters)
  : Function(parameters),
    _read_prop_user_object(nullptr),
    _column_number(getParam<unsigned int>("column_number")),
    _read_type(getParam<MooseEnum>("read_type").getEnum<PropertyReadFile::ReadTypeEnum>()),
    _point_locator(_ti_feproblem.mesh().getPointLocator())
{
  if (_column_number < _ti_feproblem.mesh().dimension() &&
      _read_type == PropertyReadFile::ReadTypeEnum::VORONOI)
    mooseWarning(
        "The column requested in the function is likely to just be containing point coordinates");
}

void
PiecewiseConstantFromCSV::initialSetup()
{
  // Get a pointer to the PropertyReadFile. A pointer is used because the UserObject is not
  // available during the construction of the function
  _read_prop_user_object = &getUserObject<PropertyReadFile>("read_prop_user_object");

  if (_read_type != _read_prop_user_object->getReadType())
    paramError("read_type", "The PropertyReadFile UO should have the same read_type parameter.");
  if (_column_number > _read_prop_user_object->getNumProperties())
    paramError("column_number",
               "Column number " + std::to_string(_column_number) +
                   " greater than total number of properties " +
                   std::to_string(_read_prop_user_object->getNumProperties()));
}

Real
PiecewiseConstantFromCSV::value(Real, const Point & p) const
{
  if (_read_type == PropertyReadFile::ReadTypeEnum::ELEMENT ||
      _read_type == PropertyReadFile::ReadTypeEnum::BLOCK)
  {
    // This is somewhat inefficient, but it allows us to retrieve the data in the
    // CSV by element or by block.
    std::set<const Elem *> candidate_elements;
    (*_point_locator)(p, candidate_elements);

    // Find the element with the lowest ID
    const Elem * min_id_elem = nullptr;
    for (const auto & elem : candidate_elements)
      if (!min_id_elem || elem->id() < min_id_elem->id())
        min_id_elem = elem;
    if (!min_id_elem)
      mooseError("No element located at ", p, " to search in element or block sorted CSV values");
    if (candidate_elements.size() > 1)
      mooseWarning("Multiple elements have been found for Point ",
                   p,
                   ". Lowest ID element will be used for reading CSV data.");

    return _read_prop_user_object->getData(min_id_elem, _column_number);
  }
  else if (_read_type == PropertyReadFile::ReadTypeEnum::NODE)
  {
    // Get the node id
    const auto node = _point_locator->locate_node(p);

    if (!node)
      mooseError("No node was found at", p, " for retrieving nodal data from CSV.");

    return _read_prop_user_object->getNodeData(node, _column_number);
  }
  else if (_read_type == PropertyReadFile::ReadTypeEnum::VORONOI)
    // No need to search for the element if we're just looking at nearest neighbors
    return _read_prop_user_object->getVoronoiData(p, _column_number);
  else
    mooseError("This should not be reachable. Implementation error somewhere");
}
