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

#ifndef GHOSTUSEROBJECT_H
#define GHOSTUSEROBJECT_H

#include "GeneralUserObject.h"

// Forward Declarations
class GhostUserObject;

template <>
InputParameters validParams<GhostUserObject>();

/**
 * User object to calculate ghosted elements on a single processor or the union across all
 * processors.
 */
class GhostUserObject : public GeneralUserObject
{
public:
  GhostUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  unsigned long getElementalValue(unsigned int element_id) const;

protected:
  std::set<dof_id_type> _ghost_data;
  dof_id_type _rank;
};

#endif // GHOSTUSEROBJECT_H
