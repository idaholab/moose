//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestXYDelaunayGeneratorArea.h"

#include "libmesh/parallel_algebra.h"

registerMooseObject("MooseTestApp", TestXYDelaunayGeneratorArea);

InputParameters
TestXYDelaunayGeneratorArea::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addRequiredParam<FunctionName>(
      "area_func", "The function evaluated at each TRI centroid to restrict area");
  return params;
}

TestXYDelaunayGeneratorArea::TestXYDelaunayGeneratorArea(const InputParameters & parameters)
  : ElementUserObject(parameters), _area_func(getFunction("area_func"))
{
}

void
TestXYDelaunayGeneratorArea::initialize()
{
  _failures.clear();
}

void
TestXYDelaunayGeneratorArea::execute()
{
  const auto area = _current_elem->volume();
  const auto centroid = _current_elem->vertex_average();
  const auto required_area = _area_func.value(_t, centroid);
  if (area > required_area)
    _failures.emplace_back(centroid, area, required_area);
}

void
TestXYDelaunayGeneratorArea::finalize()
{
  _communicator.gather(0, _failures);
  if (_failures.size())
  {
    std::stringstream out;
    out << "The following element(s) failed the area check (centroid: area > required_area):\n\n";
    out << std::scientific;
    for (const auto & [centroid, area, required_area] : _failures)
      out << "  " << centroid << ": " << area << " > " << required_area << "\n";
    mooseError(out.str());
  }
}

void
TestXYDelaunayGeneratorArea::threadJoin(const UserObject & uo)
{
  const auto & obj = static_cast<const TestXYDelaunayGeneratorArea &>(uo);
  _failures.insert(_failures.end(), obj._failures.begin(), obj._failures.end());
}
