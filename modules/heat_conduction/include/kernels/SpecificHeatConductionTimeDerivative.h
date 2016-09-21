/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SPECIFICHEATCONDUCTIONTIMEDERIVATIVE_H
#define SPECIFICHEATCONDUCTIONTIMEDERIVATIVE_H

// MOOSE includes
#include "TimeDerivative.h"
#include "Material.h"

// Forward Declarations
class SpecificHeatConductionTimeDerivative;

template<>
InputParameters validParams<SpecificHeatConductionTimeDerivative>();

/**
 * A class for defining the time derivative of the heat equation.
 *
 * By default this Kernel computes:
 *   \f$ \rho * c_p * \frac{\partial T}{\partial t}, \f$
 * where \f$ \rho \f$ and \f$ c_p \f$ are material properties for "density" and
 * "specific_heat", respectively.
 */
class SpecificHeatConductionTimeDerivative : public TimeDerivative
{
public:
  SpecificHeatConductionTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const MaterialProperty<Real> & _specific_heat;
  const MaterialProperty<Real> & _density;
};

#endif //SPECIFICHEATCONDUCTIONTIMEDERIVATIVE_H
