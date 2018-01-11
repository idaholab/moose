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

#ifndef FEBOUNDARYFLUXUSEROBJECT_H
#define FEBOUNDARYFLUXUSEROBJECT_H

// Module includes
#include "FEBoundaryBaseUserObject.h"

// Forward declarations
class FEBoundaryFluxUserObject;

template <>
InputParameters validParams<FEBoundaryFluxUserObject>();

/// This boundary variant depends on SideIntegralVariableUserObject
class FEBoundaryFluxUserObject final : public FEBoundaryBaseUserObject
{
public:
  FEBoundaryFluxUserObject(const InputParameters & parameters);

protected:
  // Override from SideIntegralVariableUserObject
  virtual Real computeQpIntegral() final;

  /// Name of the diffusion coefficient property in the local material
  const std::string _diffusion_coefficient_name;

  /// Value of the diffusion coefficient
  const MaterialProperty<Real> & _diffusion_coefficient;
};

#endif // FEBOUNDARYFLUXUSEROBJECT_H
