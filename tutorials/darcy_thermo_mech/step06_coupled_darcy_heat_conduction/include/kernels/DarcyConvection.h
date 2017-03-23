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

#ifndef DARCYCONVECTION_H
#define DARCYCONVECTION_H

#include "Kernel.h"

// Forward Declaration
class DarcyConvection;

template <>
InputParameters validParams<DarcyConvection>();

/**
 * Kernel which implements the convective term in the transient heat
 * conduction equation, and provides coupling with the Darcy pressure
 * equation.
 */
class DarcyConvection : public Kernel
{
public:
  DarcyConvection(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// The gradient of pressure
  const VariableGradient & _pressure_gradient;

  /// Coupling identifier for the pressure.  This is used to uniquely
  /// identify a coupled variable
  unsigned int _pressure_var;

  /// These references will be set by the initialization list so that
  /// values can be pulled from the Material system.
  const MaterialProperty<Real> & _permeability;
  const MaterialProperty<Real> & _porosity;
  const MaterialProperty<Real> & _viscosity;
  const MaterialProperty<Real> & _density;
  const MaterialProperty<Real> & _heat_capacity;
};

#endif // DARCYCONVECTION_H
