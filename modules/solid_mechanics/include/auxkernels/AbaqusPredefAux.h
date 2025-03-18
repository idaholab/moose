//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "AbaqusInputObjects.h"

#include <map>
#include <vector>

class AbaqusUELMesh;

/**
 * Accumulate values from one auxiliary variable into another
 */
class AbaqusPredefAux : public AuxKernel
{
public:
  static InputParameters validParams();

  AbaqusPredefAux(const InputParameters & parameters);
  void initialSetup();
  void timestepSetup();

protected:
  virtual Real computeValue();
  void update();

  AbaqusUELMesh * _uel_mesh;

  Abaqus::AbaqusID _var_id;

  // prepare data in a map for easy retrieval
  std::unordered_map<Abaqus::Index, Real> _ic_data;
};
