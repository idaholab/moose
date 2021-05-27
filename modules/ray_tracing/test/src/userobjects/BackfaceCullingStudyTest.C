//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BackfaceCullingStudyTest.h"

// Local includes
#include "TraceRay.h"

registerMooseObject("RayTracingTestApp", BackfaceCullingStudyTest);

InputParameters
BackfaceCullingStudyTest::validParams()
{
  auto params = LotsOfRaysRayStudy::validParams();
  return params;
}

BackfaceCullingStudyTest::BackfaceCullingStudyTest(const InputParameters & parameters)
  : LotsOfRaysRayStudy(parameters)
{
  // Use backface culling
  for (const auto tid : make_range(libMesh::n_threads()))
    traceRay(tid).setBackfaceCulling(true);
}

void
BackfaceCullingStudyTest::initialSetup()
{
  LotsOfRaysRayStudy::initialSetup();
  generateNormals();
}

void
BackfaceCullingStudyTest::meshChanged()
{
  LotsOfRaysRayStudy::meshChanged();
  generateNormals();
}

void
BackfaceCullingStudyTest::generateNormals()
{
  _normals.clear();

  for (const Elem * elem : _mesh.getMesh().active_element_ptr_range())
  {
    std::vector<Point> normals(elem->n_sides());
    for (const auto s : elem->side_index_range())
      normals[s] = RayTracingStudy::getSideNormal(elem, s, 0);
    _normals.emplace(elem, std::move(normals));
  }
}

const Point *
BackfaceCullingStudyTest::getElemNormals(const Elem * elem, const THREAD_ID)
{
  const auto find = _normals.find(elem);
  if (find == _normals.end())
    mooseError("Failed to find normal for elem ", elem->id());

  const auto & normals = find->second;
  mooseAssert(normals.size() == elem->n_sides(), "Normals not sized properly");
  return normals.data();
}
