/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSTHERMALINFLOWBC_H
#define NSTHERMALINFLOWBC_H

#include "NodalBC.h"


// Forward Declarations
class NSInflowThermalBC;

template<>
InputParameters validParams<NSInflowThermalBC>();

/**
 * This class is used on a boundary where the incoming flow
 * values (rho, u, v, T) are all completely specified.
 */
class NSInflowThermalBC : public NodalBC
{
public:

  NSInflowThermalBC(const std::string & name, InputParameters parameters);

  virtual ~NSInflowThermalBC(){}

protected:
  // In general, the residual equation is u-u_d=0, where u_d
  // is a Dirichlet value.  Note that no computeQpJacobian()
  // function can be specified in this class... it is assumed
  // to simply have a 1 on the diagonal.
  virtual Real computeQpResidual();

  // Specific heat at constant volume, treated as a single
  // constant value.
  Real _R;
  Real _gamma;

  // The specified density for this inflow boundary
  Real _specified_rho;

  // The specified temperature for this inflow boundary
  Real _specified_temperature;

  // The specified velocity magnitude for this inflow boundary
  Real _specified_velocity_magnitude;
};

#endif // NSTHERMALINFLOWBC_H
