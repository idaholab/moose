//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NestedDivision.h"
#include "FEProblemBase.h"
#include "libmesh/elem.h"

registerMooseObject("MooseApp", NestedDivision);

InputParameters
NestedDivision::validParams()
{
  InputParameters params = MeshDivision::validParams();
  params.addClassDescription("Divide the mesh using nested divisions objects");
  params.addRequiredParam<std::vector<MeshDivisionName>>("divisions", "Nested divisions");
  return params;
}

NestedDivision::NestedDivision(const InputParameters & parameters) : MeshDivision(parameters)
{
  for (const auto & div_name : getParam<std::vector<MeshDivisionName>>("divisions"))
    _divisions.push_back(&_fe_problem->getMeshDivision(div_name));
  if (_divisions.size() == 0)
    paramError("divisions", "You cannot pass an empty vector of divisions");
  NestedDivision::initialize();
}

void
NestedDivision::initialize()
{
  auto tot_divs = 1;
  _num_divs.resize(_divisions.size());

  for (const auto i : index_range(_divisions))
  {
    _num_divs[i] = _divisions[i]->getNumDivisions();
    tot_divs *= _num_divs[i];
    if (_num_divs[i] == 0)
      mooseError("Nested division '", _divisions[i]->name(), "' has zero bins");
  }
  setNumDivisions(tot_divs);

  // If any division does not cover the entire mesh, nested will have the same holes
  // in its coverage of the mesh
  _mesh_fully_indexed = true;
  for (const auto division : _divisions)
    if (!division->coversEntireMesh())
    {
      _mesh_fully_indexed = false;
      break;
    }
}

unsigned int
NestedDivision::divisionIndex(const Elem & elem) const
{
  unsigned int index = 0;
  unsigned int running_product = 1;
  const auto N_divs = _divisions.size();
  for (const auto i : index_range(_divisions))
  {
    index += _divisions[N_divs - 1 - i]->divisionIndex(elem) * running_product;
    running_product *= _num_divs[N_divs - 1 - i];
  }
  return index;
}

unsigned int
NestedDivision::divisionIndex(const Point & pt) const
{
  unsigned int index = 0;
  unsigned int running_product = 1;
  const auto N_divs = _divisions.size();
  for (const auto i : index_range(_divisions))
  {
    index += _divisions[N_divs - 1 - i]->divisionIndex(pt) * running_product;
    running_product *= _num_divs[N_divs - 1 - i];
  }
  return index;
}
