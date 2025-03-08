
//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AbaqusUELMesh.h"
#include "NodalUserObject.h"

class AbaqusUELMesh;

/**
 *
 */
class AbaqusUELInitialCondition : public NodalUserObject
{
public:
  static InputParameters validParams();

  AbaqusUELInitialCondition(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject &) override {}
  virtual void finalize() override;

protected:
  AbaqusUELMesh * _uel_mesh;

  // prepare IC data in a map for easy retrieval
  std::unordered_map<Abaqus::Index, std::vector<std::pair<MooseVariableField<Real> *, Real>>>
      _ic_data;
};
