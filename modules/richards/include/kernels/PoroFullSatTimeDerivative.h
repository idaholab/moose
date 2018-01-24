//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROFULLSATTIMEDERIVATIVE_H
#define POROFULLSATTIMEDERIVATIVE_H

#include "DerivativeMaterialInterface.h"
#include "TimeDerivative.h"

// Forward Declarations
class PoroFullSatTimeDerivative;

template <>
InputParameters validParams<PoroFullSatTimeDerivative>();

/**
 * Kernel = biot_coefficient*d(volumetric_strain)/dt + (1/biot_modulus)*d(porepressure)/dt
 * this is the time-derivative for poromechanics for a single-phase,
 * fully-saturated fluid with constant bulk modulus
 */
class PoroFullSatTimeDerivative : public DerivativeMaterialInterface<TimeDerivative>
{
public:
  PoroFullSatTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  /// old value of porepressure
  const VariableValue & _u_old;

  /// volumetric strain
  const MaterialProperty<Real> & _volstrain;

  /// old value of volumetric strain
  const MaterialProperty<Real> & _volstrain_old;

  /// number of displacement variables
  unsigned int _ndisp;

  /// variable number of the displacements variables
  std::vector<unsigned int> _disp_var_num;

  /// Biot coefficient
  const MaterialProperty<Real> & _alpha;

  /// 1/M, where M is the Biot modulus
  const MaterialProperty<Real> & _one_over_biot_modulus;

  /// d(1/M)/d(porepressure)
  const MaterialProperty<Real> & _done_over_biot_modulus_dP;

  /// d(1/M)/d(volumetric strain)
  const MaterialProperty<Real> & _done_over_biot_modulus_dep;
};

#endif // POROFULLSATTIMEDERIVATIVE_H
