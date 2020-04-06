//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralVariablePostprocessor.h"
#include "MaterialPropertyInterface.h"
#include "RichardsVarNames.h"

// Forward Declarations
class Function;

/**
 * Records total mass flow into an excavation defined by a RichardsExcavGeom function
 */
class RichardsExcavFlow : public SideIntegralVariablePostprocessor
{
public:
  static InputParameters validParams();

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
  const Function & _func;
};
