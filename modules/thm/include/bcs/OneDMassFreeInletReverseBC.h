#ifndef ONEDMASSFREEINLETREVERSEBC_H
#define ONEDMASSFREEINLETREVERSEBC_H

#include "OneDMassFreeBC.h"

// Forward Declarations
class OneDMassFreeInletReverseBC;

template <>
InputParameters validParams<OneDMassFreeInletReverseBC>();

/**
 * A BC for the mass equation in which nothing is specified (i.e.
 * everything is allowed to be "free") and is used for reversible
 * flow conditions at outlets.
 */
class OneDMassFreeInletReverseBC : public OneDMassFreeBC
{
public:
  OneDMassFreeInletReverseBC(const InputParameters & parameters);

protected:
  virtual bool shouldApply();

  const bool & _reversible;
  const VariableValue & _arhouA_old;
};

#endif // ONEDMASSFREEINLETREVERSEBC_H
