//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralVariableUserObject.h"

class NodalWaveSpeed : public SideIntegralVariableUserObject
{
public:
  static InputParameters validParams();

  NodalWaveSpeed(const InputParameters & parameters);
  virtual ~NodalWaveSpeed();

  virtual void threadJoin(const UserObject & uo);

  virtual void initialize();
  virtual void execute();
  virtual void finalize();

protected:
  std::map<const Node *, Real> _node_wave_speeds;

  std::map<unsigned, unsigned> _commMap;
  std::vector<Real> _commVec;

  const VariablePhiValue & _phi;

  SystemBase & _system;
  NumericVector<Number> & _aux_solution;

  const MaterialProperty<Real> & _wave_speed;
};
