//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NANINTERFACETESTFLUIDPROPERTIES_H
#define NANINTERFACETESTFLUIDPROPERTIES_H

#include "SinglePhaseFluidProperties.h"
#include "NaNInterface.h"

class NaNInterfaceTestFluidProperties;

template <>
InputParameters validParams<NaNInterfaceTestFluidProperties>();

/**
 * Fluid properties for testing NaNInterface
 */
class NaNInterfaceTestFluidProperties : public SinglePhaseFluidProperties, public NaNInterface
{
public:
  NaNInterfaceTestFluidProperties(const InputParameters & parameters);

  virtual Real p_from_v_e(Real v, Real e) const override;
  virtual void p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const override;
};

#endif /* NANINTERFACETESTFLUIDPROPERTIES_H */
