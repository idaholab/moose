/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DESORPTIONTOPORESPACE
#define DESORPTIONTOPORESPACE

#include "Kernel.h"
#include "LangmuirMaterial.h"

// Forward Declarations
class DesorptionToPorespace;

template <>
InputParameters validParams<DesorptionToPorespace>();

/**
 * Mass flow rate of fluid to the porespace from the matrix
 * Add this to the DE for the porepressure variable to get
 * fluid flowing from the matrix to the porespace
 */
class DesorptionToPorespace : public Kernel
{
public:
  DesorptionToPorespace(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// MOOSE internal variable number corresponding to the concentration in the matrix (needed for OffDiagJacobian)
  const unsigned int _conc_var;

  /// mass flow rate from matrix = mass flow rate to porespace
  const MaterialProperty<Real> & _mass_rate_from_matrix;

  /// derivative of mass flow rate from matrix wrt concentration
  const MaterialProperty<Real> & _dmass_rate_from_matrix_dC;

  /// derivative of mass flow rate from matrix wrt pressure
  const MaterialProperty<Real> & _dmass_rate_from_matrix_dp;
};

#endif // DESORPTIONTOPORESPACE
