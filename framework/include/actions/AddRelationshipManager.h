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

#ifndef ADDRELATIONSHIPMANAGER_H
#define ADDRELATIONSHIPMANAGER_H

#include "Action.h"

class AddRelationshipManager;

template <>
InputParameters validParams<AddRelationshipManager>();

/**
 * This Action retrieves all of the Actions from the MooseAction Warehouse and triggers the
 * addRelationshipManagers() call on each of them. Additionally, it is responsible for triggering
 * the attachment of those relationship managers to the relevant libMesh objects.
 */
class AddRelationshipManager : public Action
{
public:
  AddRelationshipManager(InputParameters params);

  virtual void act() override;
};

#endif // ADDRELATIONSHIPMANAGER_H
