//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GHOSTUSEROBJECTIC_H
#define GHOSTUSEROBJECTIC_H

#include "InitialCondition.h"
#include "MooseMesh.h"

class GhostUserObjectIC;
class GhostUserObject;

template <>
InputParameters validParams<GhostUserObjectIC>();

/**
 * This initial condition builds a data structure that is queried
 * for initial condition information
 */
class GhostUserObjectIC : public InitialCondition
{
public:
  GhostUserObjectIC(const InputParameters & parameters);

  virtual Real value(const Point & /*p*/);

private:
  MooseMesh & _mesh;
  std::map<dof_id_type, Real> _data;

  const GhostUserObject & _ghost_uo;
};

#endif /* GHOSTUSEROBJECTIC_H */
