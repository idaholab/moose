//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/**
 * Interface class for liquid single phase fluid properties
 */
class LiquidFluidPropertiesInterface
{
public:
  /**
   * Surface tension from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   */
  virtual Real sigma_from_p_T(Real p, Real T) const = 0;
};
