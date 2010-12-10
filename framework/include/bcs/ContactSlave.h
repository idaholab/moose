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
