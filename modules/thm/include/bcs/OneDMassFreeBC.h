#ifndef ONEDMASSFREEBC_H
#define ONEDMASSFREEBC_H

#include "OneDIntegratedBC.h"

// Forward Declarations
class OneDMassFreeBC;

template <>
InputParameters validParams<OneDMassFreeBC>();

/**
 * A BC for the mass equation in which nothing is specified (i.e.
 * everything is allowed to be "free".
 */
class OneDMassFreeBC : public OneDIntegratedBC
{
public:
  OneDMassFreeBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Coupled variables
  unsigned int _arhouA_var_number;
  const VariableValue & _arhouA;
};

#endif // ONEDMASSFREEBC_H
