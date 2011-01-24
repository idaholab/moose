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

#ifndef CONTACTSLAVE_H
#define CONTACTSLAVE_H

// MOOSE Includes
#include "BoundaryCondition.h"
#include "PenetrationLocator.h"

//Forward Declarations
class ContactSlave;

template<>
InputParameters validParams<ContactSlave>();

class ContactSlave : public BoundaryCondition
{
public:

  ContactSlave(const std::string & name, InputParameters parameters);
  
  virtual ~ContactSlave(){}

  virtual void setup();

  virtual bool shouldBeApplied();

protected:  
  virtual Real computeQpResidual();

  PenetrationLocator & _penetration_locator;
};

#endif //CONTACTSLAVE_H
