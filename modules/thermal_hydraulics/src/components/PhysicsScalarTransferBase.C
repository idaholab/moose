//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsScalarTransferBase.h"
#include "FlowModelSinglePhase.h"
#include "PhysicsFlowChannel.h"
#include "ClosuresBase.h"
#include "MooseUtils.h"
#include "ThermalHydraulicsFlowPhysics.h"

InputParameters
PhysicsScalarTransferBase::validParams()
{
  InputParameters params = ScalarTransferBase::validParams();
  params.addParam<std::vector<MooseFunctorName>>("H", "Scalar exchange coefficients [-/(m^2)]");
  return params;
}

PhysicsScalarTransferBase::PhysicsScalarTransferBase(const InputParameters & parameters)
  : ScalarTransferBase(parameters)
{
}

void
PhysicsScalarTransferBase::init()
{
  ScalarTransferBase::init();

  if (hasComponentByName<PhysicsFlowChannel>(_flow_channel_name))
  {
    const PhysicsFlowChannel & flow_channel =
        getComponentByName<PhysicsFlowChannel>(_flow_channel_name);
    for (const auto & physics : flow_channel.getPhysics())
      _th_physics.insert(physics);
    // NOTE: we currently expect to error on non thermal-hydraulics physics.
    // This may be removed in the future
  }
}

void
PhysicsScalarTransferBase::initSecondary()
{
  ScalarTransferBase::initSecondary();

  // determine names of heat transfer variables
  if (hasComponentByName<PhysicsFlowChannel>(_flow_channel_name))
  {
    const PhysicsFlowChannel & flow_channel =
        getComponentByName<PhysicsFlowChannel>(_flow_channel_name);
    const std::string Hw_suffix = flow_channel.getScalarTransferNamesSuffix(name());

    for (const auto & scalar_name : _passive_scalar_names)
      _wall_scalar_H_names.push_back("H_" + scalar_name + "_" + Hw_suffix);
  }
  else
    logError("Coupled component '", _flow_channel_name, "' must be a Physics flow channel.");
}

void
PhysicsScalarTransferBase::check() const
{
  ScalarTransferBase::check();

  if (_closures != nullptr && hasComponentByName<PhysicsFlowChannel>(_flow_channel_name))
    _closures->checkScalarTransfer(*this,
                                   getComponentByName<PhysicsFlowChannel>(_flow_channel_name));
}

void
PhysicsScalarTransferBase::addMooseObjects()
{
  ScalarTransferBase::addMooseObjects();

  _closures->addMooseObjectsScalarTransfer(
      *this, getComponentByName<PhysicsFlowChannel>(_flow_channel_name));
}

const std::string &
PhysicsScalarTransferBase::getWallScalarTransferCoefficientName(unsigned int i) const
{
  checkSetupStatus(INITIALIZED_SECONDARY);

  return _wall_scalar_H_names[i];
}
