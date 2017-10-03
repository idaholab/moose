/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ADVECTIONSUPG_H
#define ADVECTIONSUPG_H

#include "INSBase.h"

// Forward Declarations
class AdvectionSUPG;

template <>
InputParameters validParams<AdvectionSUPG>();

/**
 * This class adds SUPG contributions from the advection term of an advection-diffusion-reaction
 * problem
 */
class AdvectionSUPG : public INSBase
{
public:
  AdvectionSUPG(const InputParameters & parameters);

  virtual ~AdvectionSUPG() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned /*jvar*/) { return 0; }

  MooseEnum _tau_type;
};

#endif
