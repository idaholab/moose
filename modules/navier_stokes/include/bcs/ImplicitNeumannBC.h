/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef IMPLICITNEUMANNBC_H
#define IMPLICITNEUMANNBC_H

#include "IntegratedBC.h"

// Forward Declarations
class ImplicitNeumannBC;

template <>
InputParameters validParams<ImplicitNeumannBC>();

/**
 * This class implements a form of the Neumann boundary condition in
 * which the boundary term is treated "implicitly".  This concept is
 * discussed by Griffiths, Papanastiou, and others.
 */
class ImplicitNeumannBC : public IntegratedBC
{
public:
  ImplicitNeumannBC(const InputParameters & parameters);

  virtual ~ImplicitNeumannBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);
};

#endif // IMPLICITNEUMANNBC_H
