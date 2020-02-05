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
class TestFaceInfo;
class MooseMesh;

template <>
InputParameters validParams<TestFaceInfo>();

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
  VectorPostprocessorValue & _left_element_id;
  VectorPostprocessorValue & _right_element_id;
  VectorPostprocessorValue & _left_element_side;
  VectorPostprocessorValue & _right_element_side;
  VectorPostprocessorValue & _nx;
  VectorPostprocessorValue & _ny;
  VectorPostprocessorValue & _nz;
  VectorPostprocessorValue & _face_cx;
  VectorPostprocessorValue & _face_cy;
  VectorPostprocessorValue & _face_cz;
  VectorPostprocessorValue & _left_cx;
  VectorPostprocessorValue & _left_cy;
  VectorPostprocessorValue & _left_cz;
  VectorPostprocessorValue & _right_cx;
  VectorPostprocessorValue & _right_cy;
  VectorPostprocessorValue & _right_cz;

  std::vector<VariableName> _vars;
  std::vector<VectorPostprocessorValue *> _var_left_dof;
  std::vector<VectorPostprocessorValue *> _var_right_dof;
  std::vector<VectorPostprocessorValue *> _var_left_dof_size;
  std::vector<VectorPostprocessorValue *> _var_right_dof_size;
};
