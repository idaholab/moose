//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeParsedMaterialHelper.h"
#include "ExpressionBuilder.h"

// Forward Declarations
class EBCalcTermsTest;

template <>
InputParameters validParams<EBCalcTermsTest>();

/**
 * Grain boundary energy parameters for isotropic uniform grain boundary energies
 */
class EBCalcTermsTest : public DerivativeParsedMaterialHelper, public ExpressionBuilder
{
public:
  EBCalcTermsTest(const InputParameters & parameters);

protected:
  EBTerm _t, _tx, _ty, _tz, _txx, _txy, _txz, _tyx, _tyy, _tyz, _tzx, _tzy, _tzz;
};
