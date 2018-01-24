/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSEXCAVFLOW_H
#define RICHARDSEXCAVFLOW_H

#include "SideIntegralVariablePostprocessor.h"
#include "MaterialPropertyInterface.h"
#include "RichardsVarNames.h"

// Forward Declarations
class RichardsExcavFlow;
class Function;

template <>
InputParameters validParams<RichardsExcavFlow>();

/**
 * Records total mass flow into an excavation defined by a RichardsExcavGeom function
 */
class RichardsExcavFlow : public SideIntegralVariablePostprocessor
{
public:
  RichardsExcavFlow(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

  /// holds info regarding the Richards variables
  const RichardsVarNames & _richards_name_UO;

  /// the richards variable number for which we want the mass flow
  unsigned int _pvar;

  /// mass-flux of fluid (a vector in the multicomponent case)
  const MaterialProperty<std::vector<RealVectorValue>> & _flux;

  /// the RichardsExcavGeom that defines where on the boundary we'll compute the mass flux
  Function & _func;
};

#endif // RICHARDSEXCAVFLOW_H
