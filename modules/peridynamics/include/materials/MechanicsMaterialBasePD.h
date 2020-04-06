//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PeridynamicsMaterialBase.h"

/**
 * Base material class for peridynamic solid mechanics models
 */
class MechanicsMaterialBasePD : public PeridynamicsMaterialBase
{
public:
  static InputParameters validParams();

  MechanicsMaterialBasePD(const InputParameters & parameters);

protected:
  /**
   * Function to compute the current bond length
   */
  void computeBondCurrentLength();

  /**
   * Function to compute current bond stretch: one dimensional strain
   */
  virtual void computeBondStretch() = 0;

  ///@{ Temperature variables
  const bool _has_temp;
  MooseVariable * _temp_var;
  ///@}

  /// Bond_status variable
  MooseVariable * _bond_status_var;

  ///@{ Material properties to store
  MaterialProperty<Real> & _total_stretch;
  MaterialProperty<Real> & _mechanical_stretch;
  ///@}

  /// Displacement variables
  std::vector<MooseVariable *> _disp_var;

  /// Length of current bond
  Real _current_len;
};
