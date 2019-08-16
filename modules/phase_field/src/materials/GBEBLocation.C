//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GBEBLocation.h"

registerMooseObject("PhaseFieldApp", GBEBLocation);

template <>
InputParameters
validParams<GBEBLocation>()
{
  InputParameters params = validParams<DerivativeParsedMaterialHelper<> >();
  params.addClassDescription("Calculate value of grain boundaries in a polycrystalline sample");
  params.addParam<int>("s",20,"Sharpness of Grain Boundary edge");
  params.addParam<int>("t",20,"Thickness of Grain Boundary");
  params.addRequiredCoupledVarWithAutoBuild(
      "v","var_name_base","op_num","Array of coupled variables");

  return params;
}

GBEBLocation::GBEBLocation(const InputParameters & parameters)
  : DerivativeParsedMaterialHelper<RealVectorValue>(parameters),
    _op_num(coupledComponents("v")),
    _vals(_op_num),
    _thickness(getParam<int>("t")),
    _sharpness(getParam<int>("s"))
{
  EBTerm::EBTermVector _vals = EBTerm::CreateEBTermVector("v",_op_num);

  EBFunction _GB_location;
  _GB_location = 1.0 + pow(_vals[0],_thickness);
  for (unsigned int i = 1; i < _op_num; ++i)
    {
      _GB_location = _GB_location + pow(_vals[i],_thickness);
    }
  _GB_location = pow(-1 + 2.0/(_GB_location),_sharpness);
  EBTerm random("v0x");
  EBTerm new1("v0x");
  EBTerm new2("v0x");
  std::vector<EBTerm> random2;
  random2.push_back(random);
  random2.push_back(new1);
  random2.push_back(new2);
  EBVectorFunction random3(random2);

  EBTerm one("v0x");
  EBTerm two("v1");

  functionParse(random3);
}
