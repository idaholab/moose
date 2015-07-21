/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSGRAVITYFORCE_H
#define NSGRAVITYFORCE_H

#include "NSKernel.h"


// Forward Declarations
class NSGravityForce;

template<>
InputParameters validParams<NSGravityForce>();

class NSGravityForce : public NSKernel
{
public:

  NSGravityForce(const InputParameters & parameters);
  NSGravityForce(const std::string & deprecated_name, InputParameters parameters); // DEPRECATED CONSTRUCTOR

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  Real _acceleration;
};

#endif
