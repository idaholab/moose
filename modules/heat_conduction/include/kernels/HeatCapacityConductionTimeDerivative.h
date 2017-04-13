/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HEATCAPACITYCONDUCTIONTIMEDERIVATIVE_H
#define HEATCAPACITYCONDUCTIONTIMEDERIVATIVE_H

// MOOSE includes
#include "TimeDerivative.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class HeatCapacityConductionTimeDerivative;

template <>
InputParameters validParams<HeatCapacityConductionTimeDerivative>();

/**
 * A class for defining the time derivative of the heat equation.
 *
 * By default this Kernel computes:
 *   \f$ C_p * \frac{\partial T}{\partial t}, \f$
 * where \f$ C_p \f$ is material property for the "heat_capacity".
 */
class HeatCapacityConductionTimeDerivative
    : public DerivativeMaterialInterface<JvarMapKernelInterface<TimeDerivative>>
{
public:
  HeatCapacityConductionTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  ///@{ Heat capacity and its derivatives with respect to temperature and other coupled variables.
  const MaterialProperty<Real> & _heat_capacity;
  const MaterialProperty<Real> & _d_heat_capacity_dT;
  std::vector<const MaterialProperty<Real> *> _d_heat_capacity_dargs;
  ///@}
};

#endif // HEATCAPACITYCONDUCTIONTIMEDERIVATIVE_H
