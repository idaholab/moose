//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrystalPlasticitySlipRateGSS.h"

#include <fstream>

registerMooseObject("TensorMechanicsApp", CrystalPlasticitySlipRateGSS);

InputParameters
CrystalPlasticitySlipRateGSS::validParams()
{
  InputParameters params = CrystalPlasticitySlipRate::validParams();
  params.addParam<std::string>("uo_state_var_name",
                               "Name of state variable property: Same as "
                               "state variable user object specified in input "
                               "file.");
  params.addClassDescription("Phenomenological constitutive model slip rate class.  Override the "
                             "virtual functions in your class");
  return params;
}

CrystalPlasticitySlipRateGSS::CrystalPlasticitySlipRateGSS(const InputParameters & parameters)
  : CrystalPlasticitySlipRate(parameters),
    _mat_prop_state_var(
        getMaterialProperty<std::vector<Real>>(parameters.get<std::string>("uo_state_var_name"))),
    _pk2(getMaterialPropertyByName<RankTwoTensor>("pk2")),
    _a0(_variable_size),
    _xm(_variable_size),
    _flow_direction(getMaterialProperty<std::vector<RankTwoTensor>>(_name + "_flow_direction"))
{
  if (_slip_sys_flow_prop_file_name.length() != 0)
    readFileFlowRateParams();
  else
    getFlowRateParams();
}

void
CrystalPlasticitySlipRateGSS::readFileFlowRateParams()
{
  MooseUtils::checkFileReadable(_slip_sys_flow_prop_file_name);

  std::ifstream file;
  file.open(_slip_sys_flow_prop_file_name.c_str());

  std::vector<Real> vec;
  vec.resize(_num_slip_sys_flowrate_props);

  for (unsigned int i = 0; i < _variable_size; ++i)
  {
    for (unsigned int j = 0; j < _num_slip_sys_flowrate_props; ++j)
      if (!(file >> vec[j]))
        mooseError(
            "Error CrystalPlasticitySlipRateGSS: Premature end of slip_sys_flow_rate_param file");

    _a0(i) = vec[0];
    _xm(i) = vec[1];
  }

  file.close();
}

void
CrystalPlasticitySlipRateGSS::getFlowRateParams()
{
  if (_flowprops.size() <= 0)
    mooseError("CrystalPlasticitySlipRateGSS: Error in reading flow rate  properties: Specify "
               "input in .i file or a slip_sys_flow_prop_file_name");

  _a0.resize(_variable_size);
  _xm.resize(_variable_size);

  unsigned int num_data_grp = 2 + _num_slip_sys_flowrate_props; // Number of data per group e.g.
                                                                // start_slip_sys, end_slip_sys,
                                                                // value1, value2, ..

  for (unsigned int i = 0; i < _flowprops.size() / num_data_grp; ++i)
  {
    Real vs, ve;
    unsigned int is, ie;

    vs = _flowprops[i * num_data_grp];
    ve = _flowprops[i * num_data_grp + 1];

    if (vs <= 0 || ve <= 0)
      mooseError("CrystalPlasticitySlipRateGSS: Indices in flow rate parameter read must be "
                 "positive integers: is = ",
                 vs,
                 " ie = ",
                 ve);

    if (vs != std::floor(vs) || ve != std::floor(ve))
      mooseError("CrystalPlasticitySlipRateGSS: Error in reading flow props: Values specifying "
                 "start and end number of slip system groups should be integer");

    is = static_cast<unsigned int>(vs);
    ie = static_cast<unsigned int>(ve);

    if (is > ie)
      mooseError("CrystalPlasticitySlipRateGSS: Start index is = ",
                 is,
                 " should be greater than end index ie = ",
                 ie,
                 " in flow rate parameter read");

    for (unsigned int j = is; j <= ie; ++j)
    {
      _a0(j - 1) = _flowprops[i * num_data_grp + 2];
      _xm(j - 1) = _flowprops[i * num_data_grp + 3];
    }
  }

  for (unsigned int i = 0; i < _variable_size; ++i)
  {
    if (!(_a0(i) > 0.0 && _xm(i) > 0.0))
    {
      mooseWarning(
          "CrystalPlasticitySlipRateGSS: Non-positive flow rate parameters ", _a0(i), ",", _xm(i));
      break;
    }
  }
}

void
CrystalPlasticitySlipRateGSS::calcFlowDirection(unsigned int qp,
                                                std::vector<RankTwoTensor> & flow_direction) const
{
  DenseVector<Real> mo(LIBMESH_DIM * _variable_size), no(LIBMESH_DIM * _variable_size);

  // Update slip direction and normal with crystal orientation
  for (unsigned int i = 0; i < _variable_size; ++i)
  {
    for (const auto j : make_range(Moose::dim))
    {
      mo(i * LIBMESH_DIM + j) = 0.0;
      for (const auto k : make_range(Moose::dim))
        mo(i * LIBMESH_DIM + j) =
            mo(i * LIBMESH_DIM + j) + _crysrot[qp](j, k) * _mo(i * LIBMESH_DIM + k);
    }

    for (const auto j : make_range(Moose::dim))
    {
      no(i * LIBMESH_DIM + j) = 0.0;
      for (const auto k : make_range(Moose::dim))
        no(i * LIBMESH_DIM + j) =
            no(i * LIBMESH_DIM + j) + _crysrot[qp](j, k) * _no(i * LIBMESH_DIM + k);
    }
  }

  // Calculate Schmid tensor and resolved shear stresses
  for (unsigned int i = 0; i < _variable_size; ++i)
    for (const auto j : make_range(Moose::dim))
      for (const auto k : make_range(Moose::dim))
        flow_direction[i](j, k) = mo(i * LIBMESH_DIM + j) * no(i * LIBMESH_DIM + k);
}

bool
CrystalPlasticitySlipRateGSS::calcSlipRate(unsigned int qp, Real dt, std::vector<Real> & val) const
{
  DenseVector<Real> tau(_variable_size);

  for (unsigned int i = 0; i < _variable_size; ++i)
    tau(i) = _pk2[qp].doubleContraction(_flow_direction[qp][i]);

  for (unsigned int i = 0; i < _variable_size; ++i)
  {
    val[i] = _a0(i) * std::pow(std::abs(tau(i) / _mat_prop_state_var[qp][i]), 1.0 / _xm(i)) *
             std::copysign(1.0, tau(i));
    if (std::abs(val[i] * dt) > _slip_incr_tol)
    {
#ifdef DEBUG
      mooseWarning("Maximum allowable slip increment exceeded ", std::abs(val[i]) * dt);
#endif
      return false;
    }
  }

  return true;
}

bool
CrystalPlasticitySlipRateGSS::calcSlipRateDerivative(unsigned int qp,
                                                     Real /*dt*/,
                                                     std::vector<Real> & val) const
{
  DenseVector<Real> tau(_variable_size);

  for (unsigned int i = 0; i < _variable_size; ++i)
    tau(i) = _pk2[qp].doubleContraction(_flow_direction[qp][i]);

  for (unsigned int i = 0; i < _variable_size; ++i)
    val[i] = _a0(i) / _xm(i) *
             std::pow(std::abs(tau(i) / _mat_prop_state_var[qp][i]), 1.0 / _xm(i) - 1.0) /
             _mat_prop_state_var[qp][i];

  return true;
}
