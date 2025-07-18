//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideValueSampler.h"

// MOOSE includes
#include "MooseVariableFE.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", SideValueSampler);

InputParameters
SideValueSampler::validParams()
{
  InputParameters params = SideVectorPostprocessor::validParams();
  params += SamplerBase::validParams();
  params.addClassDescription("Sample variable(s) along a sideset, internal or external.");
  params.addRequiredCoupledVar(
      "variable", "The names of the variables that this VectorPostprocessor operates on");

  return params;
}

SideValueSampler::SideValueSampler(const InputParameters & parameters)
  : SideVectorPostprocessor(parameters),
    SamplerBase(parameters, this, _communicator),
    _qp_sampling(true)
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
SideValueSampler::initialize()
{
  SamplerBase::initialize();
}

void
SideValueSampler::execute()
{
  if (_qp_sampling)
    for (unsigned int _qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      for (unsigned int i = 0; i < _coupled_moose_vars.size(); i++)
        _values[i] = (dynamic_cast<MooseVariable *>(_coupled_moose_vars[i]))->sln()[_qp];

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
        _values[i] = MetaPhysicL::raw_value((*_fv_vars[i])(face_arg, state));
      }

      SamplerBase::addSample(fi->faceCentroid(), _current_elem->id(), _values);
    }
  }
}

void
SideValueSampler::finalize()
{
  SamplerBase::finalize();
}

void
SideValueSampler::threadJoin(const UserObject & y)
{
  const auto & vpp = static_cast<const SideValueSampler &>(y);

  SamplerBase::threadJoin(vpp);
}
