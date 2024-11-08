//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChainControlDataPostprocessor.h"
#include "MooseApp.h"
#include "ChainControlDataSystem.h"

registerMooseObject("MooseApp", ChainControlDataPostprocessor);

InputParameters
ChainControlDataPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addClassDescription("Gets a Real or bool chain control value.");

  params.addRequiredParam<std::string>("chain_control_data_name", "Chain control data name");
  return params;
}

ChainControlDataPostprocessor::ChainControlDataPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _data_name(getParam<std::string>("chain_control_data_name")),
    _real_data(nullptr),
    _bool_data(nullptr)
{
}

void
ChainControlDataPostprocessor::initialSetup()
{
  auto & chain_control_system = getMooseApp().getChainControlDataSystem();
  if (chain_control_system.hasChainControlData(_data_name))
  {
    if (chain_control_system.hasChainControlDataOfType<Real>(_data_name))
      _real_data = &chain_control_system.getChainControlData<Real>(_data_name);
    else if (chain_control_system.hasChainControlDataOfType<bool>(_data_name))
      _bool_data = &chain_control_system.getChainControlData<bool>(_data_name);
    else
      mooseError(
          "The chain control data '", _data_name, "' exists but is not of type 'Real' or 'bool'.");
  }
  else
    mooseError("The chain control data '", _data_name, "' does not exist.");
}

void
ChainControlDataPostprocessor::initialize()
{
}

void
ChainControlDataPostprocessor::execute()
{
}

Real
ChainControlDataPostprocessor::getValue() const
{
  if (_real_data)
    return _real_data->get();
  else
  {
    mooseAssert(_bool_data, "This point should not be reachable.");

    // for booleans, 'true' should convert to 1.0 and 'false' to 0.0.
    return _bool_data->get();
  }
}
