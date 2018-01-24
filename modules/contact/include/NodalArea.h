//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALAREA_H
#define NODALAREA_H

#include "SideIntegralVariableUserObject.h"

class NodalArea : public SideIntegralVariableUserObject
{
public:
  NodalArea(const InputParameters & parameters);
  virtual ~NodalArea();

  virtual void threadJoin(const UserObject & uo);

  virtual void initialize();
  virtual void execute();
  virtual void finalize();

  Real nodalArea(const Node * node) const;

protected:
  virtual Real computeQpIntegral();

  std::map<const Node *, Real> _node_areas;

  std::map<unsigned, unsigned> _commMap;
  std::vector<Real> _commVec;

  const VariablePhiValue & _phi;

  SystemBase & _system;
  NumericVector<Number> & _aux_solution;
};

template <>
InputParameters validParams<NodalArea>();

#endif
