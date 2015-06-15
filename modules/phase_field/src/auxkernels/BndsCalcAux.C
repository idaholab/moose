/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "BndsCalcAux.h"
#include "AddV.h"

template<>
InputParameters validParams<BndsCalcAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Calculate location of grain boundaries in a polycrystalline sample");
  params.addCoupledVar("v", "Array of coupled variables");
  params.addRequiredParam<unsigned int>("op_num", "number of grains");
  params.addRequiredParam<std::string>("var_name_base", "base for variable names");

  return params;
}

BndsCalcAux::BndsCalcAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, AddV(parameters) )
{
  _ncrys = coupledComponents("v");
  _vals.resize(_ncrys);

  for (unsigned int i=0; i < _ncrys; ++i)
    _vals[i] = &coupledValue("v", i);
}

Real
BndsCalcAux::computeValue()
{
  Real value = 0;

  for (unsigned int i=0; i < _ncrys; ++i)
    value += (*_vals[i])[_qp]*(*_vals[i])[_qp];

  return value;
}
