/****************************************************************/
/*             DO NOT MODIFY OR REMOVE THIS HEADER              */
/*          FALCON - Fracturing And Liquid CONvection           */
/*                                                              */
/*       (c) pending 2012 Battelle Energy Alliance, LLC         */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef VELOCITYAUX_H
#define VELOCITYAUX_H

#include "AuxKernel.h"


//Forward Declarations
class VelocityAux;

template<>
InputParameters validParams<VelocityAux>();

/**
 * Coupled auxiliary value
 */
class VelocityAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all
   * derived classes can be built using the same
   * constructor.
   */
  VelocityAux(const std::string & name, InputParameters parameters);

  virtual ~VelocityAux() {}

protected:
  virtual Real computeValue();

  MaterialProperty<RealGradient> & _darcy_flux_water;
  MaterialProperty<RealGradient> & _darcy_flux_steam;
  MaterialProperty<Real> & _porosity;
  std::string _phase;
  int _i;

};

#endif //VELOCITYAUX_H
