//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhiZeroKernel.h"

registerMooseObject("MooseTestApp", PhiZeroKernel);

InputParameters
PhiZeroKernel::validParams()
{
  InputParameters params = NullKernel::validParams();
  params.addClassDescription("Test for phi zero values");
  params.addParam<bool>("second_order", false, "use second order shape function");
  return params;
}

PhiZeroKernel::PhiZeroKernel(const InputParameters & parameters)
  : NullKernel(parameters), _second_u(_var.secondSln()), _second_phi(_var.secondPhi())
{
}

Real
PhiZeroKernel::computeQpResidual()
{

  if (_phi.size() > 0)
  {
    mooseAssert(_phi.size() == _phi_zero.size(),
                "_phi.size() " + std::to_string(_phi.size()) + "!= _phi_zero.size() " +
                    std::to_string(_phi_zero.size()));
    mooseAssert(_grad_phi.size() == _grad_phi_zero.size(),
                "_grad_phi.size() " + std::to_string(_grad_phi.size()) +
                    "!= _grad_phi_zero.size() " + std::to_string(_grad_phi_zero.size()));

    if ((_grad_phi.size() > 0) && (_phi_zero.size() > 0))
    {
      mooseAssert(_grad_phi[0].size() == _grad_phi_zero[0].size(),
                  "_grad_phi[0].size() " + std::to_string(_grad_phi_zero[0].size()) +
                      "!= _grad_phi_zero.size() " + std::to_string(_grad_phi_zero.size()));
      mooseAssert(_phi[0].size() == _phi_zero[0].size(),
                  "_phi[0].size() " + std::to_string(_phi[0].size()) + "!= _phi_zero[0].size() " +
                      std::to_string(_phi_zero[0].size()));
    }
  }
  if (getParam<bool>("second_order"))
  {
    mooseAssert(_second_phi.size() > 0,
                "_second_phi.size() = 0 with getParam<bool>(second_order)=true");
    if ((_second_phi.size() > 0))
    {
      mooseAssert(_second_phi.size() == _second_phi_zero.size(),
                  "_second_phi.size() " + std::to_string(_second_phi.size()) +
                      "!= _second_phi_zero.size() " + std::to_string(_second_phi_zero.size()));
      if (_second_phi[0].size() > 0)
      {
        mooseAssert(_second_phi[0].size() == _second_phi_zero[0].size(),
                    "_second_phi[0].size() " + std::to_string(_second_phi[0].size()) +
                        "!= _second_phi_zero[0].size() " +
                        std::to_string(_second_phi_zero[0].size()));
      }
    }
  }
  return 0.0;
}
