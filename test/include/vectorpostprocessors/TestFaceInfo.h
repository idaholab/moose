//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"
#include "Coupleable.h"

// Forward Declarations
class MooseMesh;

class TestFaceInfo : public GeneralVectorPostprocessor, public Coupleable
{
public:
  static InputParameters validParams();

  TestFaceInfo(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;

protected:
  VectorPostprocessorValue & _face_id;
  VectorPostprocessorValue & _face_area;
  VectorPostprocessorValue & _elem_element_id;
  VectorPostprocessorValue & _neighbor_element_id;
  VectorPostprocessorValue & _elem_element_side;
  VectorPostprocessorValue & _neighbor_element_side;
  VectorPostprocessorValue & _nx;
  VectorPostprocessorValue & _ny;
  VectorPostprocessorValue & _nz;
  VectorPostprocessorValue & _face_cx;
  VectorPostprocessorValue & _face_cy;
  VectorPostprocessorValue & _face_cz;
  VectorPostprocessorValue & _elem_cx;
  VectorPostprocessorValue & _elem_cy;
  VectorPostprocessorValue & _elem_cz;
  VectorPostprocessorValue & _neighbor_cx;
  VectorPostprocessorValue & _neighbor_cy;
  VectorPostprocessorValue & _neighbor_cz;

  std::vector<VariableName> _vars;
  std::vector<VectorPostprocessorValue *> _var_face_type;
};
