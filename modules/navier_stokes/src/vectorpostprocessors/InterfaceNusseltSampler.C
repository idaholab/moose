//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceNusseltSampler.h"

// MOOSE includes
#include "MooseVariableFE.h"

#include "libmesh/quadrature.h"

registerMooseObject("NavierStokesApp", InterfaceNusseltSampler);

InputParameters
InterfaceNusseltSampler::validParams()
{
  InputParameters params = SideVectorPostprocessor::validParams();
  params += SamplerBase::validParams();
  params.addClassDescription("Calculate the Nusselt number  along a sideset, internal or external.");
  params.addRequiredCoupledVar(
      "variable", "The name of the variable temperature variable that this VectorPostprocessor operates on");
  params.addRequiredParam<Real>("thermal_conductivity", "The thermal conductivity.");
  params.addRequiredParam<Real>("characteristic_length", "The characteristic length.");
  params.addRequiredParam<Real>("bulk_temperature", "The bulk temperature.");
  return params;
}

InterfaceNusseltSampler::InterfaceNusseltSampler(const InputParameters & parameters)
  : SideVectorPostprocessor(parameters),
    SamplerBase(parameters, this, _communicator),
    _qp_sampling(true),
    _thermal_conductivity(getParam<Real>("thermal_conductivity")),
    _char_length(getParam<Real>("characteristic_length")),
    _bulk_temp(getParam<Real>("bulk_temperature"))
{
  std::vector<std::string> var_names(_coupled_moose_vars.size());
  _values.resize(_coupled_moose_vars.size());

  for (unsigned int i = 0; i < _coupled_moose_vars.size(); i++)
  {
    var_names[i] = _coupled_moose_vars[i]->name();
    SamplerBase::checkForStandardFieldVariableType(_coupled_moose_vars[i]);
  }

  if (!_coupled_standard_fv_moose_vars.empty() || !_coupled_standard_linear_fv_moose_vars.empty())
  {
    const auto num_fv_vars =
        _coupled_standard_fv_moose_vars.size() + _coupled_standard_linear_fv_moose_vars.size();
    if (num_fv_vars != _coupled_moose_vars.size())
      paramError(
          "variable",
          "This object cannot accept mixed FE and FV variables, please make "
          "sure all the provided variables are either FE or FV by separating this vector "
          "postprocessor "
          "into two blocks, one for finite element and another for finite volume variables!");

    for (const auto var : _coupled_standard_fv_moose_vars)
      _fv_vars.push_back(dynamic_cast<const MooseVariableField<Real> *>(var));
    for (const auto var : _coupled_standard_linear_fv_moose_vars)
      _fv_vars.push_back(dynamic_cast<const MooseVariableField<Real> *>(var));

    _qp_sampling = false;
  }

  // Initialize the data structures in SamplerBase
  SamplerBase::setupVariables(var_names);
}

void
InterfaceNusseltSampler::initialize()
{
  SamplerBase::initialize();
}

void
InterfaceNusseltSampler::execute()
{
  if (_qp_sampling)
    for (unsigned int _qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      for (unsigned int i = 0; i < _coupled_moose_vars.size(); i++)
      {
        const Real temp_val = (dynamic_cast<MooseVariable *>(_coupled_moose_vars[i]))->sln()[_qp];
        _values[i] = -_thermal_conductivity * _thermal_conductivity *
          (dynamic_cast<MooseVariable *>(_coupled_moose_vars[i]))->gradSln()[_qp]*_normals[_qp]
          /_char_length / (temp_val - _bulk_temp);
      }

      SamplerBase::addSample(_q_point[_qp], _current_elem->id(), _values);
    }
  else
  {
    getFaceInfos();

    const auto state = determineState();

    for (const auto & fi : _face_infos)
    {
      for (unsigned int i = 0; i < _fv_vars.size(); i++)
      {
        mooseAssert(_fv_vars[i]->hasFaceSide(*fi, true) || _fv_vars[i]->hasFaceSide(*fi, false),
                    "Variable " + _fv_vars[i]->name() +
                        " should be defined on one side of the face!");

        const auto * elem = _fv_vars[i]->hasFaceSide(*fi, true) ? fi->elemPtr() : fi->neighborPtr();

        const auto face_arg = Moose::FaceArg(
            {fi, Moose::FV::LimiterType::CentralDifference, true, false, elem, nullptr});
        const Real temp_val = MetaPhysicL::raw_value((*_fv_vars[i])(face_arg, state));
        _values[i] = -_thermal_conductivity * _thermal_conductivity *
          MetaPhysicL::raw_value((*_fv_vars[i]).gradient(face_arg, state))*_normals[_qp]
          / _char_length / (temp_val - _bulk_temp)   ;
      }

      SamplerBase::addSample(fi->faceCentroid(), _current_elem->id(), _values);
    }
  }
}

void
InterfaceNusseltSampler::finalize()
{
  SamplerBase::finalize();
}

void
InterfaceNusseltSampler::threadJoin(const UserObject & y)
{
  const auto & vpp = static_cast<const InterfaceNusseltSampler &>(y);

  SamplerBase::threadJoin(vpp);
}
