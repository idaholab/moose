/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ADVECTION_H
#define ADVECTION_H

#include "INSBase.h"

// Forward Declarations
class Advection;

template <>
InputParameters validParams<Advection>();

/**
 * This class is responsible for solving the scalar advection
 * equation, possibly with a forcing function.
 */
class Advection : public INSBase
{
public:
  Advection(const InputParameters & parameters);

  virtual ~Advection() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned /*jvar*/) { return 0; }
  Function & _ffn;
  MooseEnum _tau_type;
};

#endif
