//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MixedModeEquivalentK.h"

registerMooseObject("TensorMechanicsApp", MixedModeEquivalentK);

InputParameters
MixedModeEquivalentK::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Computes the mixed-mode stress intensity factor "
                             "given the $K_I$, $K_{II}$, and $K_{III}$ stress "
                             "intensity factors");
  params.addParam<unsigned int>("ring_index", "Ring ID");
  params.addRequiredParam<VectorPostprocessorName>(
      "KI_vectorpostprocessor", "The name of the VectorPostprocessor that computes KI");
  params.addRequiredParam<VectorPostprocessorName>(
      "KII_vectorpostprocessor", "The name of the VectorPostprocessor that computes KII");
  params.addRequiredParam<VectorPostprocessorName>(
      "KIII_vectorpostprocessor", "The name of the VectorPostprocessor that computes KIII");
  params.addRequiredParam<std::string>("KI_vector_name", "The name of the vector that contains KI");
  params.addRequiredParam<std::string>("KII_vector_name",
                                       "The name of the vector that contains KII");
  params.addRequiredParam<std::string>("KIII_vector_name",
                                       "The name of the vector that contains KIII");
  params.addRequiredParam<Real>("poissons_ratio", "Poisson's ratio for the material.");
  return params;
}

MixedModeEquivalentK::MixedModeEquivalentK(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _ki_vpp_name(getParam<VectorPostprocessorName>("KI_vectorpostprocessor")),
    _kii_vpp_name(getParam<VectorPostprocessorName>("KII_vectorpostprocessor")),
    _kiii_vpp_name(getParam<VectorPostprocessorName>("KIII_vectorpostprocessor")),
    _ki_vector_name(getParam<std::string>("KI_vector_name")),
    _kii_vector_name(getParam<std::string>("KII_vector_name")),
    _kiii_vector_name(getParam<std::string>("KIII_vector_name")),
    _ki_value(getVectorPostprocessorValue("KI_vectorpostprocessor", _ki_vector_name)),
    _kii_value(getVectorPostprocessorValue("KII_vectorpostprocessor", _kii_vector_name)),
    _kiii_value(getVectorPostprocessorValue("KIII_vectorpostprocessor", _kiii_vector_name)),
    _x_value(getVectorPostprocessorValue("KI_vectorpostprocessor", "x")),
    _y_value(getVectorPostprocessorValue("KI_vectorpostprocessor", "y")),
    _z_value(getVectorPostprocessorValue("KI_vectorpostprocessor", "z")),
    _position_value(getVectorPostprocessorValue("KI_vectorpostprocessor", "id")),
    _poissons_ratio(getParam<Real>("poissons_ratio")),
    _ring_index(getParam<unsigned int>("ring_index")),
    _x(declareVector("x")),
    _y(declareVector("y")),
    _z(declareVector("z")),
    _position(declareVector("id")),
    _k_eq(declareVector("Keq_" + Moose::stringify(_ring_index)))
{
}

void
MixedModeEquivalentK::initialize()
{
  const unsigned int num_pts = _x_value.size();

  _x.assign(num_pts, 0.0);
  _y.assign(num_pts, 0.0);
  _z.assign(num_pts, 0.0);
  _position.assign(num_pts, 0.0);
  _k_eq.assign(num_pts, 0.0);
}

void
MixedModeEquivalentK::execute()
{
  for (unsigned int i = 0; i < _k_eq.size(); ++i)
  {
    _x[i] = _x_value[i];
    _y[i] = _y_value[i];
    _z[i] = _z_value[i];
    _position[i] = _position_value[i];
    _k_eq[i] = std::sqrt(_ki_value[i] * _ki_value[i] + _kii_value[i] * _kii_value[i] +
                         1.0 / (1.0 - _poissons_ratio) * _kiii_value[i] * _kiii_value[i]);
  }
}
