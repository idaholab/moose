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
#include "NonlinearSystemBase.h"
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
    // We have to declare the below strong residuals for integrity check purposes even though we may
    // not compute them. This may incur some unnecessary cost for a non-sparse derivative container
    // since when the properties are resized the entire non-sparse derivative containers will be
    // initialized to zero
    _td_strong_residual(declareADProperty<RealVectorValue>("td_strong_residual")),
    _gravity_strong_residual(declareADProperty<RealVectorValue>("gravity_strong_residual")),
    _boussinesq_strong_residual(declareADProperty<RealVectorValue>("boussinesq_strong_residual")),
    _coupled_force_strong_residual(
        declareADProperty<RealVectorValue>("coupled_force_strong_residual")),
    _convected_mesh_strong_residual(
        declareADProperty<RealVectorValue>("convected_mesh_strong_residual")),
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
    _boussinesq_alpha = &_material_data.getProperty<Real, true>(
        _object_tracker->get<MaterialPropertyName>("alpha", _current_subdomain_id), 0, *this);
    auto & temp_var = _subproblem.getStandardVariable(
        _tid, _object_tracker->get<std::string>("temperature", _current_subdomain_id));
    addMooseVariableDependency(&temp_var);
    _temperature = &temp_var.adSln();
    _ref_temp = &_material_data.getProperty<Real, false>(
        _object_tracker->get<MaterialPropertyName>("ref_temp", _current_subdomain_id), 0, *this);
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

  // Setup data for Arbitrary Lagrangian Eulerian (ALE) simulations in which the simulation domain
  // is displacing. We will need to subtract the mesh velocity from the velocity solution in order
  // to get the correct material velocity for the momentum convection term.
  if ((_has_convected_mesh =
           _object_tracker->get<bool>("has_convected_mesh", _current_subdomain_id)))
  {
    auto & disp_x = _subproblem.getStandardVariable(
        _tid, _object_tracker->get<VariableName>("disp_x", _current_subdomain_id));
    addMooseVariableDependency(&disp_x);
    _disp_x_dot = &disp_x.adUDot();
    _disp_x_sys_num = disp_x.sys().number();
    _disp_x_num = (disp_x.kind() == Moose::VarKindType::VAR_NONLINEAR) &&
                          (_disp_x_sys_num == _fe_problem.currentNonlinearSystem().number())
                      ? disp_x.number()
                      : libMesh::invalid_uint;
    if (_object_tracker->isTrackerParamValid("disp_y", _current_subdomain_id))
    {
      auto & disp_y = _subproblem.getStandardVariable(
          _tid, _object_tracker->get<VariableName>("disp_y", _current_subdomain_id));
      addMooseVariableDependency(&disp_y);
      _disp_y_dot = &disp_y.adUDot();
      _disp_y_sys_num = disp_y.sys().number();
      _disp_y_num =
          disp_y.kind() == (Moose::VarKindType::VAR_NONLINEAR &&
                            (_disp_y_sys_num == _fe_problem.currentNonlinearSystem().number()))
              ? disp_y.number()
              : libMesh::invalid_uint;
    }
    else
    {
      _disp_y_dot = nullptr;
      _disp_y_sys_num = libMesh::invalid_uint;
      _disp_y_num = libMesh::invalid_uint;
    }
    if (_object_tracker->isTrackerParamValid("disp_z", _current_subdomain_id))
    {
      auto & disp_z = _subproblem.getStandardVariable(
          _tid, _object_tracker->get<VariableName>("disp_z", _current_subdomain_id));
      addMooseVariableDependency(&disp_z);
      _disp_z_dot = &disp_z.adUDot();
      _disp_z_sys_num = disp_z.sys().number();
      _disp_z_num =
          disp_z.kind() == (Moose::VarKindType::VAR_NONLINEAR &&
                            (_disp_z_sys_num == _fe_problem.currentNonlinearSystem().number()))
              ? disp_z.number()
              : libMesh::invalid_uint;
    }
    else
    {
      _disp_z_dot = nullptr;
      _disp_z_sys_num = libMesh::invalid_uint;
      _disp_z_num = libMesh::invalid_uint;
    }
  }
  else
    _disp_x_dot = _disp_y_dot = _disp_z_dot = nullptr;

  _viscous_form = static_cast<NS::ViscousForm>(
      int(_object_tracker->get<MooseEnum>("viscous_form", _current_subdomain_id)));

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

  if (_has_convected_mesh)
  {
    ADRealVectorValue disp_dot((*_disp_x_dot)[_qp]);
    if (_disp_y_dot)
      disp_dot(1) = (*_disp_y_dot)[_qp];
    if (_disp_z_dot)
      disp_dot(2) = (*_disp_z_dot)[_qp];
    _convected_mesh_strong_residual[_qp] = -_rho[_qp] * _grad_velocity[_qp].left_multiply(disp_dot);
  }

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
}
