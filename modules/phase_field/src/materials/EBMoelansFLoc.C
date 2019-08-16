//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EBMoelansFLoc.h"

registerMooseObject("PhaseFieldApp", EBMoelansFLoc);

template <>
InputParameters
validParams<EBMoelansFLoc>()
{
  InputParameters params = validParams<DerivativeParsedMaterialHelper<> >();
  params.addClassDescription("Calculate value of grain boundaries in a polycrystalline sample");
  params.addParam<Real>("gamma",1.5,"The parameter Gamma from the EBMoelans paper, 2008");
  params.addRequiredCoupledVarWithAutoBuild(
      "v","var_name_base","op_num","Array of coupled variables");
  return params;
}

EBMoelansFLoc::EBMoelansFLoc(const InputParameters & parameters)
  : DerivativeParsedMaterialHelper<>(parameters),
    _op_num(coupledComponents("v")),
    _vals(_op_num),
    _gamma(getParam<Real>("gamma"))
{
  for (unsigned int i = 0; i < _op_num; ++i)
    {
      EBTerm _val(getVar("v",i)->name().c_str());
      _vals[i] = _val;
    }
  EBFunction _moelans_f_loc;
  _moelans_f_loc(_vals[0]) = pow(_vals[0],4) / 4 - pow(_vals[0],2) / 2;
  for (unsigned int i = 1; i < _op_num; ++i)
    {
      std::vector<EBTerm> _input_variables(_vals.begin(),_vals.begin() + i);
      std::vector<EBTerm> _output_variables(_vals.begin(),_vals.begin() + i + 1);
      _moelans_f_loc(_output_variables) = _moelans_f_loc(_input_variables) + pow(_vals[i],4) / 4 - pow(_vals[i],2) / 2;
    }
  for (unsigned int i = 0; i < _op_num; ++i)
    for (unsigned int j = 1; j < _op_num; ++j)
      if(i != j)
        _moelans_f_loc(_vals) = _moelans_f_loc(_vals) + _gamma * (_vals[i] * _vals[i] * _vals[j] * _vals[j] + 0.25);
  _moelans_f_loc(_vals) = 0.25 + _moelans_f_loc(_vals);
  functionParse(_moelans_f_loc);

}
