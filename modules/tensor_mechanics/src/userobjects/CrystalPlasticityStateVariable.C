/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CrystalPlasticityStateVariable.h"

template<>
InputParameters validParams<CrystalPlasticityStateVariable>()
{
  InputParameters params = validParams<CrystalPlasticityUOBase>();
  params.addParam<FileName>("slip_sys_res_prop_file_name", "", "Name of the file containing the initial values of slip system resistances");
  MooseEnum intvar_read_options("slip_sys_file slip_sys_res_file none","none");
  params.addParam<FileName>("slip_sys_hard_prop_file_name", "", "Name of the file containing the values of hardness evolution parameters");
  params.addParam<MooseEnum>("intvar_read_type", intvar_read_options, "Read from options for initial value of internal variables: Default from .i file");
  params.addParam<std::vector<std::string> >("uo_state_var_evol_rate_comp_name", "Name of state variable evolution rate component property: Same as state variable evolution rate component user object specified in input file.");
  params.addParam<Real>("zero", 0.0 ,"Numerical zero for interval variable");
  params.addParam<std::vector<Real> >("scale_factor", "Scale factor of individual component.");
  params.addClassDescription("Crystal plasticity state variable class.  Override the virtual functions in your class");
  return params;
}

CrystalPlasticityStateVariable::CrystalPlasticityStateVariable(const InputParameters & parameters) :
    CrystalPlasticityUOBase(parameters),
    _num_mat_state_var_evol_rate_comps(parameters.get<std::vector<std::string> >("uo_state_var_evol_rate_comp_name").size()),
    _mat_prop_state_var(getMaterialProperty<std::vector<Real> >(_name)),
    _mat_prop_state_var_old(getMaterialPropertyOld< std::vector<Real> >(_name)),
    _slip_sys_res_prop_file_name(getParam<FileName>("slip_sys_res_prop_file_name")),
    _slip_sys_hard_prop_file_name(getParam<FileName>("slip_sys_hard_prop_file_name")),
    _intvar_read_type(getParam<MooseEnum>("intvar_read_type")),
    _zero(getParam<Real>("zero")),
    _scale_factor(getParam<std::vector<Real> >("scale_factor"))
{
  if (_scale_factor.size() != _num_mat_state_var_evol_rate_comps)
    mooseError("CrystalPlasticityStateVariable: Scale factor should be have the same size of evolution rate components.");

  _mat_prop_state_var_evol_rate_comps.resize(_num_mat_state_var_evol_rate_comps);

  for (unsigned int i = 0; i < _num_mat_state_var_evol_rate_comps; ++i)
    _mat_prop_state_var_evol_rate_comps[i] = &getMaterialProperty<std::vector<Real> >(parameters.get<std::vector<std::string> >("uo_state_var_evol_rate_comp_name")[i]);
}

void
CrystalPlasticityStateVariable::initSlipSysProps(std::vector<Real> & val) const
{
  readFileInitSlipSysRes(val);
}

void
CrystalPlasticityStateVariable::readFileInitSlipSysRes(std::vector<Real> & val) const
{
  MooseUtils::checkFileReadable(_slip_sys_res_prop_file_name);

  std::ifstream file;
  file.open(_slip_sys_res_prop_file_name.c_str());

  for (unsigned int i = 0; i < _variable_size; ++i)
    if (!(file >> val[i]))
      mooseError("Error CrystalPlasticityStateVariable: Premature end of slip_sys_res_prop file");

  file.close();
}

bool
CrystalPlasticityStateVariable::updateStateVariable(unsigned int qp, Real dt, std::vector<Real> & val) const
{
  DenseVector<Real> dval(_variable_size);

  for (unsigned int i = 0; i < _variable_size; ++i)
  {
    val[i] = _mat_prop_state_var_old[qp][i];
    for (unsigned int j = 0; j < _num_mat_state_var_evol_rate_comps; j++)
      dval(i) += (*_mat_prop_state_var_evol_rate_comps[j])[qp][i] * dt * _scale_factor[j];
  }

  for (unsigned int i = 0; i < _variable_size; ++i)
  {
    if (_mat_prop_state_var_old[qp][i] < _zero && dval(i) < 0.0)
      val[i] = _mat_prop_state_var_old[qp][i];
    else
      val[i] = _mat_prop_state_var_old[qp][i] + dval(i);

    if (val[i] < 0.0)
      return false;
  }
  return true;
}
