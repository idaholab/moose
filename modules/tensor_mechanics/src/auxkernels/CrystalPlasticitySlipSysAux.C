/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CrystalPlasticitySlipSysAux.h"

template<>
InputParameters validParams<CrystalPlasticitySlipSysAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Access a component of a slip system vector property");
  params.addRequiredParam<std::string>("slipsysvar", "The slip system variable name");
  params.addRequiredRangeCheckedParam<unsigned int>("index_i", "index_i != 0", "The slip system i");
  return params;
}

CrystalPlasticitySlipSysAux::CrystalPlasticitySlipSysAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _slipsysvar(getMaterialProperty<std::vector<Real> >(getParam<std::string>("slipsysvar"))),
    _i(getParam<unsigned int>("index_i"))
{
}

Real
CrystalPlasticitySlipSysAux::computeValue()
{
  return _slipsysvar[_qp][_i-1];
}
