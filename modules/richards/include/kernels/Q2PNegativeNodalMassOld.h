/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef Q2PNEGATIVENODALMASSOLD
#define Q2PNEGATIVENODALMASSOLD

#include "TimeKernel.h"
#include "Material.h"
#include "RichardsDensity.h"

// Forward Declarations
class Q2PNegativeNodalMassOld;

template <>
InputParameters validParams<Q2PNegativeNodalMassOld>();

/**
 * -fluid_mass_old/dt with the fluid mass
 * being lumped to the nodes.
 */
class Q2PNegativeNodalMassOld : public TimeKernel
{
public:
  Q2PNegativeNodalMassOld(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  const RichardsDensity & _density;

  /// old value of the other variable (this is porepressure if the Variable is saturation)
  const VariableValue & _other_var_nodal_old;

  /// whether the "other variable" is actually porepressure
  bool _var_is_pp;

  /// value of the porosity at the start of the timestep
  const MaterialProperty<Real> & _porosity_old;
};

#endif // Q2PNEGATIVENODALMASSOLD
