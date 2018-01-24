//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrystalPlasticityRotationOutAux.h"

#include <fstream>

template <>
InputParameters
validParams<CrystalPlasticityRotationOutAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription(
      "Output updated rotation tensor to a file: Use for stereographic plots");
  params.addParam<FileName>(
      "rotout_file_name", "rot.out", "Name of rotation output file: Default rot.out");
  params.addParam<unsigned int>("output_frequency", 1, "Frequency of Output");
  return params;
}

CrystalPlasticityRotationOutAux::CrystalPlasticityRotationOutAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _rotout_file_name(getParam<FileName>("rotout_file_name")),
    _out_freq(getParam<unsigned int>("output_frequency")),
    _update_rot(getMaterialProperty<RankTwoTensor>("update_rot"))
{
}

Real
CrystalPlasticityRotationOutAux::computeValue()
{
  std::ofstream fileout;

  if (_t_step % _out_freq == 0)
  {
    fileout.open(_rotout_file_name.c_str(), std::ofstream::out | std::ofstream::app);
    fileout << _t_step << ' ' << _dt << ' ' << _JxW[_qp] << ' ' << _update_rot[_qp](0, 0) << ' '
            << _update_rot[_qp](0, 1) << ' ' << _update_rot[_qp](0, 2) << ' '
            << _update_rot[_qp](1, 0) << ' ' << _update_rot[_qp](1, 1) << ' '
            << _update_rot[_qp](1, 2) << ' ' << _update_rot[_qp](2, 0) << ' '
            << _update_rot[_qp](2, 1) << ' ' << _update_rot[_qp](2, 2) << '\n';
  }

  return 0;
}
