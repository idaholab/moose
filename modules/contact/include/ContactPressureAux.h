#ifndef CONTACTPRESSUREAUX_H
#define CONTACTPRESSUREAUX_H

#include "AuxKernel.h"

class NodalArea;
class PenetrationLocator;

class ContactPressureAux : public AuxKernel
{
public:

  ContactPressureAux(const std::string & name, InputParameters parameters);

  virtual ~ContactPressureAux();

protected:
  virtual Real computeValue();

  const VariableValue & _nodal_area;
  const PenetrationLocator & _penetration_locator;
};

template<>
InputParameters validParams<ContactPressureAux>();

#endif
