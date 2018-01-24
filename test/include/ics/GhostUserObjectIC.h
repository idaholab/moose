/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
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
