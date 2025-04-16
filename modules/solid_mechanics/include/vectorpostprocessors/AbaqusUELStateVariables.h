//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"
#include "AbaqusInputObjects.h"

class AbaqusUELMeshUserElement;

/**
 *
 */
class AbaqusUELStateVariables : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  AbaqusUELStateVariables(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  const AbaqusUELMeshUserElement & _uel;
  const std::vector<Abaqus::Index> _active_elements;
  const std::vector<Abaqus::Element> & _uel_elements;
  VectorPostprocessorValue & _id_vector;
  std::vector<VectorPostprocessorValue *> _value_vectors;
  const unsigned int _split;
  VectorPostprocessorValue * const _split_vector;
};
