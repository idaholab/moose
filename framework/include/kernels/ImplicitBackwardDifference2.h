/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef IMPLICITBD2
#define IMPLICITBD2

#include "Kernel.h"

// Forward Declarations
class ImplicitBackwardDifference2;
template<>
InputParameters validParams<ImplicitBackwardDifference2>();

class ImplicitBackwardDifference2 : public Kernel
{
public:

  ImplicitBackwardDifference2(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  bool _start_with_be;
};
#endif //IMPLICITBD2
