//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVectorFEWave.h"

registerKokkosResidualObject("MooseTestApp", KokkosVectorFEWave);

InputParameters
KokkosVectorFEWave::validParams()
{
  InputParameters params = Moose::Kokkos::KernelCurl::validParams();
  params.addRequiredParam<FunctionName>("x_forcing_func", "The x forcing function.");
  params.addParam<FunctionName>("y_forcing_func", "The y forcing function.");
  params.addParam<FunctionName>("z_forcing_func", "The z forcing function.");
  return params;
}

KokkosVectorFEWave::KokkosVectorFEWave(const InputParameters & parameters)
  : Moose::Kokkos::KernelCurl(parameters),
    _has_y_ffn(isParamValid("y_forcing_func")),
    _has_z_ffn(isParamValid("z_forcing_func")),
    _x_ffn(getKokkosFunction<KokkosParsedFunction>("x_forcing_func")),
    _y_ffn(
        getKokkosFunction<KokkosParsedFunction>(_has_y_ffn ? "y_forcing_func" : "x_forcing_func")),
    _z_ffn(
        getKokkosFunction<KokkosParsedFunction>(_has_z_ffn ? "z_forcing_func" : "x_forcing_func"))
{
}
