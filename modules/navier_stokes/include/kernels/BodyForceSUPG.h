/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef BODYFORCESUPG_H
#define BODYFORCESUPG_H

#include "INSBase.h"

// Forward Declarations
class BodyForceSUPG;

template <>
InputParameters validParams<BodyForceSUPG>();

/**
 * This class adds SUPG contributions from the source term of an advection-diffusion-reaction
 * problem
 */
class BodyForceSUPG : public INSBase
{
public:
  BodyForceSUPG(const InputParameters & parameters);

  virtual ~BodyForceSUPG() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned /*jvar*/) { return 0; }

  Function & _function;
  MooseEnum _tau_type;
};

#endif
