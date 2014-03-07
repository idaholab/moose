#ifndef NSMASSINVISCIDFLUX_H
#define NSMASSINVISCIDFLUX_H

#include "NSKernel.h"


// Forward Declarations
class NSMassInviscidFlux;

template<>
InputParameters validParams<NSMassInviscidFlux>();

class NSMassInviscidFlux : public NSKernel
{
public:

  NSMassInviscidFlux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
};

#endif
