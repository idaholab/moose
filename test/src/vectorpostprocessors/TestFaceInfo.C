//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestFaceInfo.h"

// MOOSE includes
#include "MooseMesh.h"

registerMooseObject("MooseTestApp", TestFaceInfo);

InputParameters
TestFaceInfo::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addParam<std::vector<VariableName>>("vars", "Variable names");
  params.addClassDescription("Computes element face quatities like area, neighbors, normals, etc.");
  return params;
}

TestFaceInfo::TestFaceInfo(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    Coupleable(this, false),
    _face_id(declareVector("id")),
    _face_area(declareVector("area")),
    _elem_element_id(declareVector("elem_elem")),
    _neighbor_element_id(declareVector("neighbor_elem")),
    _elem_element_side(declareVector("elem_side")),
    _neighbor_element_side(declareVector("neighbor_side")),
    _nx(declareVector("nx")),
    _ny(declareVector("ny")),
    _nz(declareVector("nz")),
    _face_cx(declareVector("face_cx")),
    _face_cy(declareVector("face_cy")),
    _face_cz(declareVector("face_cz")),
    _elem_cx(declareVector("elem_cx")),
    _elem_cy(declareVector("elem_cy")),
    _elem_cz(declareVector("elem_cz")),
    _neighbor_cx(declareVector("neighbor_cx")),
    _neighbor_cy(declareVector("neighbor_cy")),
    _neighbor_cz(declareVector("neighbor_cz"))
{
  if (isParamValid("vars"))
  {
    _vars = getParam<std::vector<VariableName>>("vars");
    for (auto & v : _vars)
      _var_face_type.push_back(&declareVector(v + "_face_type"));
  }
}

void
TestFaceInfo::execute()
{
  unsigned int j = 0;
  for (auto & p : _fe_problem.mesh().faceInfo())
  {
    _face_id.push_back(j);
    _face_area.push_back(p->faceArea());
    _elem_element_id.push_back(p->elem().id());
    _elem_element_side.push_back(p->elemSideID());

    Point normal = p->normal();
    _nx.push_back(normal(0));
    _ny.push_back(normal(1));
    _nz.push_back(normal(2));
    Point fc = p->faceCentroid();
    _face_cx.push_back(fc(0));
    _face_cy.push_back(fc(1));
    _face_cz.push_back(fc(2));
    Point lc = p->elemCentroid();
    _elem_cx.push_back(lc(0));
    _elem_cy.push_back(lc(1));
    _elem_cz.push_back(lc(2));
    if (p->neighborPtr())
    {
      _neighbor_element_id.push_back(p->neighbor().id());
      _neighbor_element_side.push_back(p->neighborSideID());
      Point rc = p->neighborCentroid();
      _neighbor_cx.push_back(rc(0));
      _neighbor_cy.push_back(rc(1));
      _neighbor_cz.push_back(rc(2));
    }
    else
    {
      _neighbor_element_id.push_back(-1);
      _neighbor_element_side.push_back(-1);
      _neighbor_cx.push_back(std::numeric_limits<double>::max());
      _neighbor_cy.push_back(std::numeric_limits<double>::max());
      _neighbor_cz.push_back(std::numeric_limits<double>::max());
    }

    for (unsigned int l = 0; l < _vars.size(); ++l)
    {
      FaceInfo::VarFaceNeighbors vfn = p->faceType(_vars[l]);
      Real x = 0;
      switch (vfn)
      {
        case FaceInfo::VarFaceNeighbors::BOTH:
          x = 1;
          break;
        case FaceInfo::VarFaceNeighbors::ELEM:
          x = 2;
          break;
        case FaceInfo::VarFaceNeighbors::NEIGHBOR:
          x = 3;
          break;
        case FaceInfo::VarFaceNeighbors::NEITHER:
          x = 4;
          break;
      }
      _var_face_type[l]->push_back(x);
    }
    ++j;
  }
}
