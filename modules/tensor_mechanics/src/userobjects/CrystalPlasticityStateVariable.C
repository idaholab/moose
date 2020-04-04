//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrystalPlasticityStateVariable.h"

#include <fstream>

registerMooseObject("TensorMechanicsApp", CrystalPlasticityStateVariable);

InputParameters
CrystalPlasticityStateVariable::validParams()
{
  InputParameters params = CrystalPlasticityUOBase::validParams();
  params.addParam<FileName>(
      "state_variable_file_name",
      "",
      "Name of the file containing the initial values of slip system resistances");
  MooseEnum intvar_read_options("file_input inline_input user_input", "inline_input");
  params.addParam<MooseEnum>(
      "intvar_read_type",
      intvar_read_options,
      "Read from options for initial value of internal variables: Default from .i file");
  params.addParam<std::vector<unsigned int>>("groups",
                                             "To group the initial values on different "
                                             "slip systems 'format: [start end)', i.e.'0 "
                                             "4 8 11' groups 0-3, 4-7 and 8-11 ");
  params.addParam<std::vector<Real>>("group_values",
                                     "The initial values corresponding to each "
                                     "group, i.e. '0.0 1.0 2.0' means 0-4 = 0.0, "
                                     "4-8 = 1.0 and 8-12 = 2.0 ");
  params.addParam<std::vector<std::string>>("uo_state_var_evol_rate_comp_name",
                                            "Name of state variable evolution rate component "
                                            "property: Same as state variable evolution rate "
                                            "component user object specified in input file.");
  params.addParam<Real>("zero", 0.0, "Numerical zero for interval variable");
  params.addParam<std::vector<Real>>("scale_factor", "Scale factor of individual component.");
  params.addClassDescription(
      "Crystal plasticity state variable class.  Override the virtual functions in your class");
  return params;
}

CrystalPlasticityStateVariable::CrystalPlasticityStateVariable(const InputParameters & parameters)
  : CrystalPlasticityUOBase(parameters),
    _num_mat_state_var_evol_rate_comps(
        parameters.get<std::vector<std::string>>("uo_state_var_evol_rate_comp_name").size()),
    _mat_prop_state_var(getMaterialProperty<std::vector<Real>>(_name)),
    _state_variable_file_name(getParam<FileName>("state_variable_file_name")),
    _intvar_read_type(getParam<MooseEnum>("intvar_read_type")),
    _groups(getParam<std::vector<unsigned int>>("groups")),
    _group_values(getParam<std::vector<Real>>("group_values")),
    _zero(getParam<Real>("zero")),
    _scale_factor(getParam<std::vector<Real>>("scale_factor"))
{
  if (_scale_factor.size() != _num_mat_state_var_evol_rate_comps)
    mooseError("CrystalPlasticityStateVariable: Scale factor should be have the same size of "
               "evolution rate components.");

  _mat_prop_state_var_evol_rate_comps.resize(_num_mat_state_var_evol_rate_comps);

  for (unsigned int i = 0; i < _num_mat_state_var_evol_rate_comps; ++i)
    _mat_prop_state_var_evol_rate_comps[i] = &getMaterialProperty<std::vector<Real>>(
        parameters.get<std::vector<std::string>>("uo_state_var_evol_rate_comp_name")[i]);
}

void
CrystalPlasticityStateVariable::initSlipSysProps(std::vector<Real> & val,
                                                 const Point & q_point) const
{
  switch (_intvar_read_type)
  {
    case 0:
      readInitialValueFromFile(val);
      break;
    case 1:
      readInitialValueFromInline(val);
      break;
    case 2:
      provideInitialValueByUser(val, q_point);
      break;
    default:
      mooseError("CrystalPlasticityStateVariable: Read option for initial value of internal "
                 "variables is not supported.");
  }

  for (unsigned int i = 0; i < _variable_size; ++i)
    if (val[i] <= 0.0)
      mooseError("CrystalPlasticityStateVariable: Value of state variables ", i, " non positive");
}

void
CrystalPlasticityStateVariable::readInitialValueFromFile(std::vector<Real> & val) const
{
  MooseUtils::checkFileReadable(_state_variable_file_name);

  std::ifstream file;
  file.open(_state_variable_file_name.c_str());

  for (unsigned int i = 0; i < _variable_size; ++i)
    if (!(file >> val[i]))
      mooseError("Error CrystalPlasticityStateVariable: Premature end of state_variable file");

  file.close();
}

void
CrystalPlasticityStateVariable::readInitialValueFromInline(std::vector<Real> & val) const
{
  if (_groups.size() <= 0)
    mooseError("CrystalPlasticityStateVariable: Error in reading initial state variable values: "
               "Specify input in .i file or in state_variable file");
  else if (_groups.size() != (_group_values.size() + 1))
    mooseError(
        "CrystalPlasticityStateVariable: The size of the groups and group_values does not match.");

  for (unsigned int i = 0; i < _groups.size() - 1; ++i)
  {
    unsigned int is, ie;

    is = _groups[i];
    ie = _groups[i + 1] - 1;

    if (is > ie)
      mooseError("CrystalPlasticityStateVariable: Start index is = ",
                 is,
                 " should be greater than end index ie = ",
                 ie,
                 " in state variable read");

    for (unsigned int j = is; j <= ie; ++j)
      val[j] = _group_values[i];
  }
}

void
CrystalPlasticityStateVariable::provideInitialValueByUser(std::vector<Real> & /*val*/,
                                                          const Point & /*q_point*/) const
{
  mooseError("Error CrystalPlasticityStateVariable: User has to overwrite "
             "'provideInitialValueByUser' function"
             "in order to provide specific initial values based on quadrature point location.");
}

bool
CrystalPlasticityStateVariable::updateStateVariable(unsigned int qp,
                                                    Real dt,
                                                    std::vector<Real> & val,
                                                    std::vector<Real> & val_old) const
{
  for (unsigned int i = 0; i < _variable_size; ++i)
  {
    val[i] = 0.0;
    for (unsigned int j = 0; j < _num_mat_state_var_evol_rate_comps; j++)
      val[i] += (*_mat_prop_state_var_evol_rate_comps[j])[qp][i] * dt * _scale_factor[j];
  }

  for (unsigned int i = 0; i < _variable_size; ++i)
  {
    if (val_old[i] < _zero && val[i] < 0.0)
      val[i] = val_old[i];
    else
      val[i] = val_old[i] + val[i];

    if (val[i] < 0.0)
      return false;
  }
  return true;
}
