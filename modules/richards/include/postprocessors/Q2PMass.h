/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef Q2PMASS_H
#define Q2PMASS_H

#include "ElementIntegralVariablePostprocessor.h"
#include "RichardsDensity.h"

//Forward Declarations
class Q2PMass;

template<>
InputParameters validParams<Q2PMass>();

/**
 * This postprocessor computes the fluid mass by integrating the porosity*density*S over the volume
 * Here S = saturation, if variable=saturation and _var_is_pp=false
 * Here S = 1 - saturation, if variable=porepressure and _var_is_pp=true
 *
 */
class Q2PMass: public ElementIntegralVariablePostprocessor
{
public:
  Q2PMass(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

  /// porisity
  const MaterialProperty<Real> & _porosity;

  /// density
  const RichardsDensity & _density;

  /// the other variable
  VariableValue & _other_var;

  /// whether to use S = saturation or S = (1-saturation)
  bool _var_is_pp;
};

#endif
