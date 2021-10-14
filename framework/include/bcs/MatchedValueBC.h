//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericNodalBC.h"

/**
 * Implements a simple coupled boundary condition where u=v on the boundary.
 */
template <bool is_ad>
class MatchedValueBCTempl : public GenericNodalBC<is_ad>
{
public:
  static InputParameters validParams();

  MatchedValueBCTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const GenericVariableValue<is_ad> & _v;

  /// The id of the coupled variable
  unsigned int _v_num;

  /// General members
  usingGenericNodalBCMembers;

private:
  /// Coefficient for primary variable
  const Real _u_coeff;
  /// Coefficient for coupled variable
  const Real _v_coeff;
};

using MatchedValueBC = MatchedValueBCTempl<false>;
using ADMatchedValueBC = MatchedValueBCTempl<true>;
