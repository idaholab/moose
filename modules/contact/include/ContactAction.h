/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CONTACTACTION_H
#define CONTACTACTION_H

#include "Action.h"
#include "MooseTypes.h"
#include "MooseEnum.h"

class ContactAction;

template <>
InputParameters validParams<ContactAction>();

class ContactAction : public Action
{
public:
  ContactAction(const InputParameters & params);

  virtual void act() override;

protected:
  const BoundaryName _master;
  const BoundaryName _slave;
  const std::string _model;
  const std::string _formulation;
  const MooseEnum _system;
};

#endif // CONTACTACTION_H
