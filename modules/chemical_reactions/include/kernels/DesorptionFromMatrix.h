/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DESORPTIONFROMMATRIX
#define DESORPTIONFROMMATRIX

#include "Kernel.h"
#include "LangmuirMaterial.h"

// Forward Declarations
class DesorptionFromMatrix;

template <>
InputParameters validParams<DesorptionFromMatrix>();

/**
 * Mass flow rate of adsorbed fluid from matrix
 * Add this to TimeDerivative to form the entire DE for desorption of fluid-in-the-matrix
 */
class DesorptionFromMatrix : public Kernel
{
public:
  DesorptionFromMatrix(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// MOOSE internal variable number corresponding to the porepressure (need this of OffDiagJacobian)
  const unsigned int _pressure_var;

  /// mass flow rate from matrix = mass flow rate to porespace
  const MaterialProperty<Real> & _mass_rate_from_matrix;

  /// derivative of mass flow rate from matrix wrt concentration
  const MaterialProperty<Real> & _dmass_rate_from_matrix_dC;

  /// derivative of mass flow rate from matrix wrt pressure
  const MaterialProperty<Real> & _dmass_rate_from_matrix_dp;
};

#endif // DESORPTIONFROMMATRIX
