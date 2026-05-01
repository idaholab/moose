//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SharpInterfacePressureCoupledVelocityError.h"

#include "Function.h"
#include "MooseMesh.h"
#include "SharpInterfaceRhieChowMassFlux.h"
#include "libmesh/elem.h"

registerMooseObject("NavierStokesApp", SharpInterfacePressureCoupledVelocityError);

InputParameters
SharpInterfacePressureCoupledVelocityError::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum component("x y z", "x");
  params.addRequiredParam<UserObjectName>("rhie_chow_user_object",
                                          "The sharp-interface Rhie-Chow user object.");
  params.addRequiredParam<MooseEnum>(
      "component", component, "The velocity component to compare.");
  params.addRequiredParam<FunctionName>("exact_velocity",
                                        "The exact velocity function for the selected component.");
  params.addClassDescription("Computes an L2 error for the reconstructed sharp-interface "
                             "pressure-coupled cell velocity correction against the exact "
                             "correction needed to recover the manufactured final velocity from "
                             "the current predictor branch.");
  return params;
}

SharpInterfacePressureCoupledVelocityError::SharpInterfacePressureCoupledVelocityError(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(_subproblem.mesh()),
    _component(getParam<MooseEnum>("component") == "y" ? 1
                  : getParam<MooseEnum>("component") == "z" ? 2
                                                             : 0),
    _exact_velocity(getFunction("exact_velocity")),
    _rhie_chow(getUserObject<SharpInterfaceRhieChowMassFlux>("rhie_chow_user_object")),
    _error(0.0)
{
}

void
SharpInterfacePressureCoupledVelocityError::initialize()
{
  _error = 0.0;
}

void
SharpInterfacePressureCoupledVelocityError::execute()
{
  const auto state = determineState();

  for (const Elem * elem : _mesh.getMesh().active_local_element_ptr_range())
  {
    const auto & elem_info = _mesh.elemInfo(elem->id());
    const Real approx =
        _rhie_chow.pressureCoupledCellVelocityDelta(elem_info, state)(_component);
    const Real predictor = _rhie_chow.predictorVelocityComponent(elem_info, _component);
    const Real exact =
        _exact_velocity.value(_t, elem_info.centroid()) - predictor;
    const Real diff = approx - exact;
    _error += diff * diff * elem_info.volume() * elem_info.coordFactor();
  }
}

void
SharpInterfacePressureCoupledVelocityError::finalize()
{
  _communicator.sum(_error);
  _error = std::sqrt(_error);
}

Real
SharpInterfacePressureCoupledVelocityError::getValue() const
{
  return _error;
}
