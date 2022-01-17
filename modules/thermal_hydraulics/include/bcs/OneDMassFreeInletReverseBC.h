#pragma once

#include "OneDMassFreeBC.h"

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

public:
  static InputParameters validParams();
};
