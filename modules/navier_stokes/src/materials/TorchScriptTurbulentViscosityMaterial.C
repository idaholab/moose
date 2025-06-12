//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "TorchScriptTurbulentViscosityMaterial.h"

registerMooseObject("MooseApp", TorchScriptTurbulentViscosityMaterial);

InputParameters
TorchScriptTurbulentViscosityMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Material object which relies on the evaluation of a TorchScript module to compute the turbulent dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<UserObjectName>(
      "torch_script_userobject",
      "The name of the user object which contains the torch script module.");
  params.addParam<Real>("k", "k value for Reynold's stress");
  params.addParam<bool>("debug", "Value to control debugging console messages");

  return params;
}

TorchScriptTurbulentViscosityMaterial::TorchScriptTurbulentViscosityMaterial(const InputParameters & parameters)
  : Material(parameters),
    _mesh_dimension(_mesh.dimension()),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(parameters.isParamValid("v") ? &(getFunctor<ADReal>("v")) : nullptr),
    _w_var(parameters.isParamValid("w") ? &(getFunctor<ADReal>("w")) : nullptr),
    _k(getParam<Real>("k")),
    _debug(getParam<bool>("debug")),
    _torch_script_userobject(getUserObject<TorchScriptUserObject>("torch_script_userobject")),
    _input_tensor(torch::zeros(
        {1, 2},
        torch::TensorOptions().dtype(torch::kFloat64).device(_app.getLibtorchDevice())))
    {
    _properties = &declareGenericPropertyByName<Real, false>("mu_t_torch");
}

void
TorchScriptTurbulentViscosityMaterial::initQpStatefulProperties()
{
  computeQpValues();
}

void
TorchScriptTurbulentViscosityMaterial::computeQpProperties()
{
  computeQpValues();
}

void
TorchScriptTurbulentViscosityMaterial::computeQpValues()
{
    
  const auto r = makeElemArg(_current_elem);
  const auto t = determineState();

  Real eta_1 = 0.0;
  Real eta_2 = 0.0;
  Real tr_sij = 0.0;

  const auto u_grad_x = _u_var.gradient(r,t)(0);
  eta_1 += MetaPhysicL::raw_value(Utility::pow<2>(0.5 * (u_grad_x + u_grad_x)));
  eta_2 += MetaPhysicL::raw_value(Utility::pow<2>(0.5 * (u_grad_x - u_grad_x)));
  tr_sij += MetaPhysicL::raw_value(u_grad_x);

  if (_mesh_dimension > 1)
  {
    const auto u_grad_y = _u_var.gradient(r,t)(1);
    const auto v_grad_x = _v_var->gradient(r,t)(0);
    const auto v_grad_y = _v_var->gradient(r,t)(1);

    eta_1 += MetaPhysicL::raw_value(Utility::pow<2>(0.5 * (u_grad_y + v_grad_x)));
    eta_1 += MetaPhysicL::raw_value(Utility::pow<2>(0.5 * (v_grad_x + u_grad_y)));
    eta_1 += MetaPhysicL::raw_value(Utility::pow<2>(0.5 * (v_grad_y + v_grad_y)));

    eta_2 += MetaPhysicL::raw_value(Utility::pow<2>(0.5 * (u_grad_y - v_grad_x)));
    eta_2 += MetaPhysicL::raw_value(Utility::pow<2>(0.5 * (v_grad_x - u_grad_y)));
    eta_2 += MetaPhysicL::raw_value(Utility::pow<2>(0.5 * (v_grad_y - v_grad_y)));

    tr_sij += MetaPhysicL::raw_value(v_grad_y);
  }
  if (_mesh_dimension > 2)
  {
    const auto u_grad_z = _u_var.gradient(r,t)(2);
    const auto v_grad_z = _v_var->gradient(r,t)(2);
    const auto w_grad_x = _v_var->gradient(r,t)(0);
    const auto w_grad_y = _v_var->gradient(r,t)(1);
    const auto w_grad_z = _v_var->gradient(r,t)(2);

    eta_1 += MetaPhysicL::raw_value(Utility::pow<2>(0.5 * (u_grad_z + w_grad_x)));
    eta_1 += MetaPhysicL::raw_value(Utility::pow<2>(0.5 * (w_grad_x + u_grad_z)));
    eta_1 += MetaPhysicL::raw_value(Utility::pow<2>(0.5 * (v_grad_z + w_grad_y)));
    eta_1 += MetaPhysicL::raw_value(Utility::pow<2>(0.5 * (w_grad_y + v_grad_z)));
    eta_1 += MetaPhysicL::raw_value(Utility::pow<2>(0.5 * (w_grad_z + w_grad_z)));

    eta_2 += MetaPhysicL::raw_value(Utility::pow<2>(0.5 * (u_grad_z - w_grad_x)));
    eta_2 += MetaPhysicL::raw_value(Utility::pow<2>(0.5 * (w_grad_x - u_grad_z)));
    eta_2 += MetaPhysicL::raw_value(Utility::pow<2>(0.5 * (v_grad_z - w_grad_y)));
    eta_2 += MetaPhysicL::raw_value(Utility::pow<2>(0.5 * (w_grad_y - v_grad_z)));
    eta_2 += MetaPhysicL::raw_value(Utility::pow<2>(0.5 * (w_grad_z - w_grad_z)));

    tr_sij += MetaPhysicL::raw_value(w_grad_z);
  }

  auto input_accessor = _input_tensor.accessor<Real, 2>();
  input_accessor[0][0] =  std::max(eta_1, 0.1);
  input_accessor[0][1] =  std::max(eta_2, 0.1);

  const auto output = _torch_script_userobject.evaluate(_input_tensor);
  const auto output_accessor = output.accessor<Real, 2>();

  const Real nu_t = std::max(output_accessor[0][0] + 2./3.*_k/input_accessor[0][0]*std::abs(tr_sij), 1e-3);

  if (_debug)
  {
    _console << "eta_1: " << input_accessor[0][0] << " eta_2: " << input_accessor[0][0] << std::endl;
    _console << "nu_t output: " << nu_t << std::endl;
  }
  
  (*_properties)[_qp] = nu_t;
}

#endif
