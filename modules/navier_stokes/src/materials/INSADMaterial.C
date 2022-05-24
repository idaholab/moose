//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMaterial.h"
#include "Function.h"
#include "Assembly.h"
#include "INSADObjectTracker.h"
#include "FEProblemBase.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSADMaterial);

InputParameters
INSADMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("This is the material class used to compute some of the strong "
                             "residuals for the INS equations.");
  params.addRequiredCoupledVar("velocity", "The velocity");
  params.addRequiredCoupledVar(NS::pressure, "The pressure");
  params.addParam<MaterialPropertyName>("mu_name", "mu", "The name of the dynamic viscosity");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");
  return params;
}

INSADMaterial::INSADMaterial(const InputParameters & parameters)
  : Material(parameters),
    _velocity(adCoupledVectorValue("velocity")),
    _grad_velocity(adCoupledVectorGradient("velocity")),
    _grad_p(adCoupledGradient(NS::pressure)),
    _mu(getADMaterialProperty<Real>("mu_name")),
    _rho(getADMaterialProperty<Real>("rho_name")),
    _velocity_dot(nullptr),
    _mass_strong_residual(declareADProperty<Real>("mass_strong_residual")),
    _advective_strong_residual(declareADProperty<RealVectorValue>("advective_strong_residual")),
    _viscous_strong_residual(declareADProperty<RealVectorValue>("viscous_strong_residual")),
    // We have to declare the below strong residuals for integrity check purposes even though we may
    // not compute them. This may incur some unnecessary cost for a non-sparse derivative container
    // since when the properties are resized the entire non-sparse derivative containers will be
    // initialized to zero
    _td_strong_residual(declareADProperty<RealVectorValue>("td_strong_residual")),
    _gravity_strong_residual(declareADProperty<RealVectorValue>("gravity_strong_residual")),
    _boussinesq_strong_residual(declareADProperty<RealVectorValue>("boussinesq_strong_residual")),
    _coupled_force_strong_residual(
        declareADProperty<RealVectorValue>("coupled_force_strong_residual")),
    // _mms_function_strong_residual(declareProperty<RealVectorValue>("mms_function_strong_residual")),
    _use_displaced_mesh(getParam<bool>("use_displaced_mesh")),
    _ad_q_point(_bnd ? _assembly.adQPointsFace() : _assembly.adQPoints()),
    _rz_radial_coord(_mesh.getAxisymmetricRadialCoord()),
    _rz_axial_coord(_rz_radial_coord == 0 ? 1 : 0)
{
  if (!_fe_problem.hasUserObject("ins_ad_object_tracker"))
  {
    InputParameters tracker_params = INSADObjectTracker::validParams();
    tracker_params.addPrivateParam("_moose_app", &_app);

    _fe_problem.addUserObject("INSADObjectTracker", "ins_ad_object_tracker", tracker_params);
  }

  // Bypass the UserObjectInterface method because it requires a UserObjectName param which we
  // don't need
  _object_tracker = &_fe_problem.getUserObject<INSADObjectTracker>("ins_ad_object_tracker");
  const_cast<INSADObjectTracker *>(_object_tracker)->addBlockIDs(this->blockIDs());
}

void
INSADMaterial::subdomainSetup()
{
  if ((_has_transient = _object_tracker->get<bool>("has_transient", _current_subdomain_id)))
    _velocity_dot = &adCoupledVectorDot("velocity");
  else
    _velocity_dot = nullptr;

  if ((_has_boussinesq = _object_tracker->get<bool>("has_boussinesq", _current_subdomain_id)))
  {
    // Material property retrieval through MaterialPropertyInterface APIs can only happen during
    // object contruction because we're going to check for material property dependency resolution.
    // So we have to go through MaterialData here. We already performed the material property
    // requests through the MaterialPropertyInterface APIs in the INSAD kernels, so we should be
    // safe for dependencies
    _boussinesq_alpha = &_material_data->getADProperty<Real>(
        _object_tracker->get<MaterialPropertyName>("alpha", _current_subdomain_id));
    _temperature =
        &_subproblem
             .getStandardVariable(
                 _tid, _object_tracker->get<std::string>("temperature", _current_subdomain_id))
             .adSln();
    _ref_temp = &_material_data->getProperty<Real>(
        _object_tracker->get<MaterialPropertyName>("ref_temp", _current_subdomain_id));
  }
  else
  {
    _boussinesq_alpha = nullptr;
    _temperature = nullptr;
    _ref_temp = nullptr;
  }

  _has_gravity = _object_tracker->get<bool>("has_gravity", _current_subdomain_id);
  if (_has_gravity || _has_boussinesq)
    _gravity_vector = _object_tracker->get<RealVectorValue>("gravity", _current_subdomain_id);
  else
    _gravity_vector = 0;

  _viscous_form = static_cast<std::string>(
      _object_tracker->get<MooseEnum>("viscous_form", _current_subdomain_id));

  if ((_has_coupled_force = _object_tracker->get<bool>("has_coupled_force", _current_subdomain_id)))
  {
    _coupled_force_var.clear();
    _coupled_force_vector_function.clear();
    if (_object_tracker->isTrackerParamValid("coupled_force_var", _current_subdomain_id))
    {
      const auto & var_names = _object_tracker->get<std::vector<VariableName>>(
          "coupled_force_var", _current_subdomain_id);
      for (const auto & var_name : var_names)
        _coupled_force_var.push_back(&_subproblem.getVectorVariable(_tid, var_name).adSln());
    }

    if (_object_tracker->isTrackerParamValid("coupled_force_vector_function",
                                             _current_subdomain_id))
    {
      const auto & func_names = _object_tracker->get<std::vector<FunctionName>>(
          "coupled_force_vector_function", _current_subdomain_id);
      for (const auto & func_name : func_names)
        _coupled_force_vector_function.push_back(&_fe_problem.getFunction(func_name, _tid));
    }
  }
}

void
INSADMaterial::computeQpProperties()
{
  _mass_strong_residual[_qp] = -_grad_velocity[_qp].tr();
  if (_coord_sys == Moose::COORD_RZ)
    // Subtract u_r / r
    _mass_strong_residual[_qp] -=
        _velocity[_qp](_rz_radial_coord) / (_use_displaced_mesh ? _ad_q_point[_qp](_rz_radial_coord)
                                                                : _q_point[_qp](_rz_radial_coord));

  _advective_strong_residual[_qp] = _rho[_qp] * _grad_velocity[_qp] * _velocity[_qp];
  if (_has_transient)
    _td_strong_residual[_qp] = _rho[_qp] * (*_velocity_dot)[_qp];
  if (_has_gravity)
    _gravity_strong_residual[_qp] = -_rho[_qp] * _gravity_vector;
  if (_has_boussinesq)
    _boussinesq_strong_residual[_qp] = (*_boussinesq_alpha)[_qp] * _gravity_vector * _rho[_qp] *
                                       ((*_temperature)[_qp] - (*_ref_temp)[_qp]);
  if (_has_coupled_force)
  {
    _coupled_force_strong_residual[_qp] = 0;
    mooseAssert(!(_coupled_force_var.empty() && _coupled_force_vector_function.empty()),
                "Either the coupled force var or the coupled force vector function must be "
                "non-empty in 'INSADMaterial'");
    for (const auto * var : _coupled_force_var)
    {
      mooseAssert(var, "null coupled variable in INSADMaterial");
      _coupled_force_strong_residual[_qp] -= (*var)[_qp];
    }
    for (const auto * fn : _coupled_force_vector_function)
    {
      mooseAssert(fn, "null coupled function in INSADMaterial");
      _coupled_force_strong_residual[_qp] -= fn->vectorValue(_t, _q_point[_qp]);
    }
  }

  // // Future Addition
  // _mms_function_strong_residual[_qp] = -RealVectorValue(_x_vel_fn.value(_t, _q_point[_qp]),
  //                                                       _y_vel_fn.value(_t, _q_point[_qp]),
  //                                                       _z_vel_fn.value(_t, _q_point[_qp]));

  // // The code immediately below is fictional. E.g. there is no Moose::Laplacian nor is there
  // // currently a _second_velocity member because TypeNTensor (where N = 3 in this case) math is not
  // // really implemented in libMesh. Hence we cannot add this strong form contribution of the viscous
  // // term at this time. Note that for linear elements this introduces no error in the consistency of
  // // stabilization methods, and in general for bi-linear elements, the error introduced is small
  // _viscous_strong_residual[_qp] = -_mu[_qp] * Moose::Laplacian(_second_velocity[_qp]);

  if (_coord_sys == Moose::COORD_RZ)
    viscousTermRZ();
}

void
INSADMaterial::viscousTermRZ()
{
  // To understand the code immediately below, visit
  // https://en.wikipedia.org/wiki/Del_in_cylindrical_and_spherical_coordinates.
  // The u_r / r^2 term comes from the vector Laplacian. The -du_i/dr * 1/r term comes from
  // the scalar Laplacian. The scalar Laplacian in axisymmetric cylindrical coordinates is
  // equivalent to the Cartesian Laplacian plus a 1/r * du_i/dr term. And of course we are
  // applying a minus sign here because the strong form is -\nabala^2 * \vec{u}
  //
  // Another note: libMesh implements grad(v) as dvi/dxj

  if (_use_displaced_mesh)
  {
    ADReal r = _ad_q_point[_qp](_rz_radial_coord);

    if (_viscous_form == "laplace")
      _viscous_strong_residual[_qp] = ADRealVectorValue(
          // u_r
          // Additional term from vector Laplacian
          _mu[_qp] * (_velocity[_qp](_rz_radial_coord) / (r * r) -
                      // Additional term from scalar Laplacian
                      _grad_velocity[_qp](_rz_radial_coord, _rz_radial_coord) / r),
          // u_z
          // Additional term from scalar Laplacian
          -_mu[_qp] * _grad_velocity[_qp](_rz_axial_coord, _rz_radial_coord) / r,
          0);
    else
      _viscous_strong_residual[_qp] =
          ADRealVectorValue(2. * _mu[_qp] *
                                (_velocity[_qp](_rz_radial_coord) / (r * r) -
                                 _grad_velocity[_qp](_rz_radial_coord, _rz_radial_coord) / r),
                            -_mu[_qp] / r * (_grad_velocity[_qp](1, 0) + _grad_velocity[_qp](0, 1)),
                            0);
  }
  else
  {
    Real r = _q_point[_qp](_rz_radial_coord);
    if (_viscous_form == "laplace")
      _viscous_strong_residual[_qp] =
          // u_r
          // Additional term from vector Laplacian
          ADRealVectorValue(
              _mu[_qp] * (_velocity[_qp](_rz_radial_coord) /
                              (_q_point[_qp](_rz_radial_coord) * _q_point[_qp](_rz_radial_coord)) -
                          // Additional term from scalar Laplacian
                          _grad_velocity[_qp](_rz_radial_coord, _rz_radial_coord) /
                              _q_point[_qp](_rz_radial_coord)),
              // u_z
              // Additional term from scalar Laplacian
              -_mu[_qp] * _grad_velocity[_qp](_rz_axial_coord, _rz_radial_coord) /
                  _q_point[_qp](_rz_radial_coord),
              0);
    else
      _viscous_strong_residual[_qp] =
          ADRealVectorValue(2. * _mu[_qp] *
                                (_velocity[_qp](_rz_radial_coord) / (r * r) -
                                 _grad_velocity[_qp](_rz_radial_coord, _rz_radial_coord) / r),
                            -_mu[_qp] / r * (_grad_velocity[_qp](1, 0) + _grad_velocity[_qp](0, 1)),
                            0);
  }
}
