//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GHOSTUSEROBJECT_H
#define GHOSTUSEROBJECT_H

#include "ElementUserObject.h"

// Forward Declarations
class GhostUserObject;

template <>
InputParameters validParams<GhostUserObject>();

/**
 * User object to calculate ghosted elements on a single processor or the union across all
 * processors.
 */
class GhostUserObject : public ElementUserObject
{
public:
  GhostUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject &) override;

  Real getElementalValue(dof_id_type element_id) const;

protected:
  std::map<dof_id_type, Real> _ghost_data;
  dof_id_type _rank;

  const VariableValue & _variable;
};

#endif // GHOSTUSEROBJECT_H
