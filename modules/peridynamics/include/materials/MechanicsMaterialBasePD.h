//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MaterialBasePD.h"

class MechanicsMaterialBasePD;

template <>
InputParameters validParams<MechanicsMaterialBasePD>();

/**
 * Base material class for peridynamic solid mechanics models
 */
class MechanicsMaterialBasePD : public MaterialBasePD
{
public:
  MechanicsMaterialBasePD(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;

  /**
   * Function to compute the current length of current bond
   */
  void computeBondCurrentLength();

  /**
   * Function to compute current bond stretch: one dimensional strain
   */
  virtual void computeBondStretch() = 0;

  ///@{ Temperature variables
  const bool _has_temp;
  MooseVariableFEBase * _temp_var;
  ///@}

  /// Bond_status variable
  MooseVariableFEBase & _bond_status_var;

  ///@{ Material properties to store
  MaterialProperty<Real> & _total_stretch;
  MaterialProperty<Real> & _mechanical_stretch;
  ///@}

  /// Displacement variables
  std::vector<MooseVariableFEBase *> _disp_var;

  /// Length of current bond
  Real _current_length;
};
