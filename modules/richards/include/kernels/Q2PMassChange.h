/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef Q2PMASSCHANGE
#define Q2PMASSCHANGE

#include "TimeKernel.h"
#include "Material.h"
#include "RichardsDensity.h"

// Forward Declarations
class Q2PMassChange;

template<>
InputParameters validParams<Q2PMassChange>();

/**
 * d(fluid mass in porespace)/dt with the fluid mass
 * being lumped to the nodes.
 */
class Q2PMassChange : public TimeKernel
{
public:

  Q2PMassChange(const InputParameters & parameters);

protected:

  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const RichardsDensity & _density;

  /// the other variable (this is porepressure if the Variable is saturation)
  VariableValue & _other_var_nodal;

  /// old value of the other variable (this is porepressure if the Variable is saturation)
  VariableValue & _other_var_nodal_old;

  /// variable number of the other variable
  unsigned int _other_var_num;

  /// whether the "other variable" is actually porepressure
  bool _var_is_pp;

  /// current value of the porosity
  const MaterialProperty<Real> & _porosity;

  /// value of the porosity at the start of the timestep
  const MaterialProperty<Real> & _porosity_old;
};

#endif //Q2PMASSCHANGE
