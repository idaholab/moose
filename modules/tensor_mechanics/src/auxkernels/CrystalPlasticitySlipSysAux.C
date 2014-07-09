#include "CrystalPlasticitySlipSysAux.h"

template<>
InputParameters validParams<CrystalPlasticitySlipSysAux>()
{
  InputParameters params = validParams<AuxKernel>();
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
