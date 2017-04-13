/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef Q2PNODALMASS
#define Q2PNODALMASS

#include "TimeKernel.h"
#include "Material.h"
#include "RichardsDensity.h"

// Forward Declarations
class Q2PNodalMass;

template <>
InputParameters validParams<Q2PNodalMass>();

/**
 * fluid_mass/dt lumped to the nodes
 */
class Q2PNodalMass : public TimeKernel
{
public:
  Q2PNodalMass(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const RichardsDensity & _density;

  /// the other variable (this is porepressure if the Variable is saturation)
  const VariableValue & _other_var_nodal;

  /// variable number of the other variable
  unsigned int _other_var_num;

  /// whether the "other variable" is actually porepressure
  bool _var_is_pp;

  /// current value of the porosity
  const MaterialProperty<Real> & _porosity;
};

#endif // Q2PNODALMASS
