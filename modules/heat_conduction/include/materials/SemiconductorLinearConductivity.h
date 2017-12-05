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

#ifndef SEMICONDUCTORLINEARCONDUCTIVITY_H
#define SEMICONDUCTORLINEARCONDUCTIVITY_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class SemiconductorLinearConductivity;

template <>
InputParameters validParams<SemiconductorLinearConductivity>();
/**
 * Calculates resistivity and electrical conductivity as a function of temperature.
 * For semiconductor, Steinhart-Hart equation describes the temperature dependence
 * of the resistivity: 1/T = A + B ln (\rho) + C (ln (\rho))^3
 *
 * For linear relationship ONLY => C = 0
 *      1/T = A -B ln(\sigma)
 *   \rho: electrical resistivity in ohm-m = 1/(\sigma)
 *   \sigma: electrical conductivity in 1/(ohm-m)
 */
class SemiconductorLinearConductivity : public DerivativeMaterialInterface<Material>
{
public:
  SemiconductorLinearConductivity(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  const Real _sh_coeff_A;
  const Real _sh_coeff_B;
  const VariableValue & _T;

  std::string _base_name;
  MaterialProperty<Real> & _electric_conductivity;
  MaterialProperty<Real> & _delectric_conductivity_dT;
};

#endif // SEMICONDUCTORLINEARCONDUCTIVITY_H
