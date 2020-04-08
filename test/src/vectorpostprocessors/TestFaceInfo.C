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

registerMooseObject("MooseApp", TestFaceInfo);

defineLegacyParams(TestFaceInfo);

InputParameters
TestFaceInfo::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addParam<std::vector<VariableName>>("vars", "Variable names");
  return params;
}

TestFaceInfo::TestFaceInfo(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    Coupleable(this, false),
    _face_id(declareVector("id")),
    _face_area(declareVector("area")),
    _left_element_id(declareVector("left_elem")),
    _right_element_id(declareVector("right_elem")),
    _left_element_side(declareVector("left_side")),
    _right_element_side(declareVector("right_side")),
    _nx(declareVector("nx")),
    _ny(declareVector("ny")),
    _nz(declareVector("nz")),
    _face_cx(declareVector("face_cx")),
    _face_cy(declareVector("face_cy")),
    _face_cz(declareVector("face_cz")),
    _left_cx(declareVector("left_cx")),
    _left_cy(declareVector("left_cy")),
    _left_cz(declareVector("left_cz")),
    _right_cx(declareVector("right_cx")),
    _right_cy(declareVector("right_cy")),
    _right_cz(declareVector("right_cz"))
{
  if (isParamValid("vars"))
  {
    _vars = getParam<std::vector<VariableName>>("vars");
    for (auto & v : _vars)
    {
      _var_left_dof.push_back(&declareVector(v + "_left"));
      _var_right_dof.push_back(&declareVector(v + "_right"));
      _var_left_dof_size.push_back(&declareVector(v + "_size_left"));
      _var_right_dof_size.push_back(&declareVector(v + "_size_right"));
    }
  }
}

void
TestFaceInfo::execute()
{
  unsigned int j = 0;
  for (auto & p : _fe_problem.mesh().faceInfo())
  {
    _face_id.push_back(j);
    _face_area.push_back(p.faceArea());
    _left_element_id.push_back(p.leftElem().id());
    _left_element_side.push_back(p.leftSideID());
    // the right element might be a nullptr
    if (!p.rightElemPtr())
      _right_element_id.push_back(Elem::invalid_id);
    else
      _right_element_id.push_back(p.rightElem().id());

    Point normal = p.normal();
    _nx.push_back(normal(0));
    _ny.push_back(normal(1));
    _nz.push_back(normal(2));
    Point fc = p.faceCentroid();
    _face_cx.push_back(fc(0));
    _face_cy.push_back(fc(1));
    _face_cz.push_back(fc(2));
    Point lc = p.leftCentroid();
    _left_cx.push_back(lc(0));
    _left_cy.push_back(lc(1));
    _left_cz.push_back(lc(2));
    Point rc = p.rightCentroid();
    _right_cx.push_back(rc(0));
    _right_cy.push_back(rc(1));
    _right_cz.push_back(rc(2));

    for (unsigned int l = 0; l < _vars.size(); ++l)
    {
      auto & dofs = p.leftDofIndices(_vars[l]);
      _var_left_dof[l]->push_back(dofs[0]);
      _var_left_dof_size[l]->push_back(dofs.size());
      dofs = p.rightDofIndices(_vars[l]);
      _var_right_dof[l]->push_back(dofs[0]);
      _var_right_dof_size[l]->push_back(dofs.size());
    }
    ++j;
  }
}
