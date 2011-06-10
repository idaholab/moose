#ifndef NSMOMENTUMVISCOUSFLUX_H
#define NSMOMENTUMVISCOUSFLUX_H

#include "NSViscousFluxBase.h"


// ForwardDeclarations
class NSMomentumViscousFlux;

template<>
InputParameters validParams<NSMomentumViscousFlux>();


/**
 * Derived instance of the NSViscousFluxBase class
 * for the momentum equations.
 */
class NSMomentumViscousFlux : public NSViscousFluxBase
{
public:

  NSMomentumViscousFlux(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // Required parameter
  unsigned _component;
};
 
#endif //  NSMOMENTUMVISCOUSFLUX_H
