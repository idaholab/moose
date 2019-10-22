//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EBCalcTermsTest.h"
#include <iostream>

registerMooseObject("PhaseFieldApp", EBCalcTermsTest);

template <>
InputParameters
validParams<EBCalcTermsTest>()
{
  InputParameters params = validParams<DerivativeParsedMaterialHelper>();
  params.addClassDescription("Test to see if calculus components of coupled variables can be used "
                             "(gradient & second derivative terms) in ExpressionBuilder");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of other coupled order parameters");

  return params;
}

EBCalcTermsTest::EBCalcTermsTest(const InputParameters & parameters)
  : DerivativeParsedMaterialHelper(parameters),
    _t("v0"),
    _tx("v0x"),
    _ty("v0y"),
    _tz("v0z"),
    _txx("v0xx"),
    _txy("v0xy"),
    _txz("v0xz"),
    _tyx("v0yx"),
    _tyy("v0yy"),
    _tyz("v0yz"),
    _tzx("v0zx"),
    _tzy("v0zy"),
    _tzz("v0zz")
{
  EBFunction tester;
  std::vector<EBTerm> EBTermsVector;
  EBTermsVector.push_back(_t);
  EBTermsVector.push_back(_tx);
  EBTermsVector.push_back(_ty);
  EBTermsVector.push_back(_tz);
  EBTermsVector.push_back(_txx);
  EBTermsVector.push_back(_txy);
  EBTermsVector.push_back(_txz);
  EBTermsVector.push_back(_tyx);
  EBTermsVector.push_back(_tyy);
  EBTermsVector.push_back(_tyz);
  EBTermsVector.push_back(_tzx);
  EBTermsVector.push_back(_tzy);
  EBTermsVector.push_back(_tzz);

  tester(EBTermsVector) = pow(_t, 2) + pow(_tx, 2) + pow(_ty, 2) + pow(_tz, 2) + pow(_txx, 2) +
                          pow(_txy, 2) + pow(_txz, 2) + pow(_tyx, 2) + pow(_tyy, 2) + pow(_tyz, 2) +
                          pow(_tzx, 2) + pow(_tzy, 2) + pow(_tzz, 2);

  functionParse(tester);
}
