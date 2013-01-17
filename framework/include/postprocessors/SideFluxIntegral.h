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

#ifndef SIDEFLUXINTEGRAL_H
#define SIDEFLUXINTEGRAL_H

#include "SideIntegralVariablePostprocessor.h"
#include "MaterialPropertyInterface.h"

//Forward Declarations
class SideFluxIntegral;

template<>
InputParameters validParams<SideFluxIntegral>();

/**
 * This postprocessor computes a side integral of the mass flux.
 */
class SideFluxIntegral : public SideIntegralVariablePostprocessor
{
public:
  SideFluxIntegral(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpIntegral();

  std::string _diffusivity;
  MaterialProperty<Real> & _diffusion_coef;
};

#endif // SIDEFLUXINTEGRAL_H
