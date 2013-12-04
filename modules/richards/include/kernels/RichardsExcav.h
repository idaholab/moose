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

#ifndef RICHARDSEXCAV
#define RICHARDSEXCAV

#include "NodalBC.h"

// Forward Declarations
class RichardsExcav;
class Function;

template<>
InputParameters validParams<RichardsExcav>();

class RichardsExcav : public NodalBC
{
public:

  RichardsExcav(const std::string & name,
                        InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual bool shouldApply();

  Real _p_excav;
  Function & _func;
};

#endif //RICHARDSEXCAV

