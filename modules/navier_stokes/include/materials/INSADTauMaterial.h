//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "NonlinearSystemBase.h"
#include "FEProblemBase.h"
#include "MaterialProperty.h"
#include "MooseArray.h"
#include "INSADMaterial.h"
#include "NavierStokesMethods.h"
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "MooseMesh.h"

#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/fe_type.h"

#include <vector>

class INSADMaterial;

template <typename T>
class INSADTauMaterialTempl : public T
{
public:
  static InputParameters validParams();

  INSADTauMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;
  virtual void computeQpProperties() override;

  /**
   * Compute the maximum dimension of the current element
   */
  void computeHMax();

  /**
   * Compute the viscous strong residual
   */
  void computeViscousStrongResidual();

  /**
   * Compute the strong form corresponding to RZ pieces of the viscous term
   */
  void viscousTermRZ();

  /**
   * Whether to seed with respect to velocity derivatives
   */
  bool doVelocityDerivatives() const;

  const Real _alpha;
  ADMaterialProperty<Real> & _tau;

  /// Strong residual corresponding to the momentum viscous term. This is only used by stabilization
  /// kernels
  ADMaterialProperty<RealVectorValue> & _viscous_strong_residual;

  /// The strong residual of the momentum equation
  ADMaterialProperty<RealVectorValue> & _momentum_strong_residual;

  ADReal _hmax;

  /// The velocity variable
  const VectorMooseVariable * const _velocity_var;

  /// A scalar Lagrange FE data member to compute the velocity second derivatives since
  /// they're currently not supported for vector FE types
  const FEBase * const & _scalar_lagrange_fe;

  /// Containers to hold the matrix of second spatial derivatives of velocity
  std::vector<ADRealTensorValue> _d2u;
  std::vector<ADRealTensorValue> _d2v;
  std::vector<ADRealTensorValue> _d2w;

  /// The velocity variable number
  const unsigned int _vel_number;

  /// The velocity system number
  const unsigned int _vel_sys_number;

  using T::_ad_q_point;
  using T::_advective_strong_residual;
  using T::_assembly;
  using T::_boussinesq_strong_residual;
  using T::_convected_mesh_strong_residual;
  using T::_coord_sys;
  using T::_coupled_force_strong_residual;
  using T::_current_elem;
  using T::_disp_x_num;
  using T::_disp_x_sys_num;
  using T::_disp_y_num;
  using T::_disp_y_sys_num;
  using T::_disp_z_num;
  using T::_disp_z_sys_num;
  using T::_dt;
  using T::_fe_problem;
  using T::_grad_p;
  using T::_grad_velocity;
  using T::_gravity_strong_residual;
  using T::_has_boussinesq;
  using T::_has_convected_mesh;
  using T::_has_coupled_force;
  using T::_has_gravity;
  using T::_has_transient;
  using T::_mesh;
  using T::_mu;
  using T::_object_tracker;
  using T::_q_point;
  using T::_qp;
  using T::_qrule;
  using T::_rho;
  using T::_rz_axial_coord;
  using T::_rz_radial_coord;
  using T::_td_strong_residual;
  using T::_use_displaced_mesh;
  using T::_velocity;
  using T::_viscous_form;
  using T::getVectorVar;
};

typedef INSADTauMaterialTempl<INSADMaterial> INSADTauMaterial;

template <typename T>
InputParameters
INSADTauMaterialTempl<T>::validParams()
{
  InputParameters params = T::validParams();
  params.addClassDescription(
      "This is the material class used to compute the stabilization parameter tau.");
  params.addParam<Real>("alpha", 1., "Multiplicative factor on the stabilization parameter tau.");
  return params;
}

template <typename T>
INSADTauMaterialTempl<T>::INSADTauMaterialTempl(const InputParameters & parameters)
  : T(parameters),
    _alpha(this->template getParam<Real>("alpha")),
    _tau(this->template declareADProperty<Real>("tau")),
    _viscous_strong_residual(
        this->template declareADProperty<RealVectorValue>("viscous_strong_residual")),
    _momentum_strong_residual(
        this->template declareADProperty<RealVectorValue>("momentum_strong_residual")),
    _velocity_var(getVectorVar("velocity", 0)),
    _scalar_lagrange_fe(
        _assembly.getFE(FEType(_velocity_var->feType().order, LAGRANGE), _mesh.dimension())),
    _vel_number(_velocity_var->number()),
    _vel_sys_number(_velocity_var->sys().number())
{
  _scalar_lagrange_fe->get_d2phi();
}

template <typename T>
bool
INSADTauMaterialTempl<T>::doVelocityDerivatives() const
{
  return ADReal::do_derivatives &&
         (_vel_sys_number == _fe_problem.currentNonlinearSystem().number());
}

template <typename T>
void
INSADTauMaterialTempl<T>::computeHMax()
{
  if (_disp_x_num == libMesh::invalid_uint || !ADReal::do_derivatives)
  {
    _hmax = _current_elem->hmax();
    return;
  }

  _hmax = 0;
  std::array<unsigned int, 3> disps = {_disp_x_num, _disp_y_num, _disp_z_num};
  std::array<unsigned int, 3> disp_sys_nums = {_disp_x_sys_num, _disp_y_sys_num, _disp_z_sys_num};

  for (unsigned int n_outer = 0; n_outer < _current_elem->n_vertices(); n_outer++)
    for (unsigned int n_inner = n_outer + 1; n_inner < _current_elem->n_vertices(); n_inner++)
    {
      VectorValue<DualReal> diff = (_current_elem->point(n_outer) - _current_elem->point(n_inner));
      for (const auto i : index_range(disps))
      {
        const auto disp_num = disps[i];
        if (disp_num == libMesh::invalid_uint)
          continue;
        const auto sys_num = disp_sys_nums[i];

        // Here we insert derivatives of the difference in nodal positions with respect to the
        // displacement degrees of freedom. From above, diff = outer_node_position -
        // inner_node_position
        diff(i).derivatives().insert(
            _current_elem->node_ref(n_outer).dof_number(sys_num, disp_num, 0)) = 1.;
        diff(i).derivatives().insert(
            _current_elem->node_ref(n_inner).dof_number(sys_num, disp_num, 0)) = -1.;
      }

      _hmax = std::max(_hmax, diff.norm_sq());
    }

  _hmax = std::sqrt(_hmax);
}

template <typename T>
void
INSADTauMaterialTempl<T>::computeProperties()
{
  computeHMax();
  computeViscousStrongResidual();

  T::computeProperties();
}

template <typename T>
void
INSADTauMaterialTempl<T>::computeViscousStrongResidual()
{
  auto resize_and_zero = [this](auto & d2vel)
  {
    d2vel.resize(_qrule->n_points());
    for (auto & d2qp : d2vel)
      d2qp = 0;
  };
  resize_and_zero(_d2u);
  resize_and_zero(_d2v);
  resize_and_zero(_d2w);

  auto get_d2 = [this](const auto i) -> std::vector<ADRealTensorValue> &
  {
    switch (i)
    {
      case 0:
        return _d2u;
      case 1:
        return _d2v;
      case 2:
        return _d2w;
      default:
        mooseError("invalid value of 'i'");
    }
  };

  // libMesh does not yet have the capability for computing second order spatial derivatives of
  // vector bases. Lacking that capability, we can compute the second order spatial derivatives "by
  // hand" using the scalar field version of the vector basis, e.g. LAGRANGE instead of
  // LAGRANGE_VEC. Adding this implementation allows us to be fully consistent with results from a
  // scalar velocity field component implementation of Navier-Stokes
  const auto & vel_dof_indices = _velocity_var->dofIndices();
  for (const auto i : index_range(vel_dof_indices))
  {
    // This may not work if the element and spatial dimensions are different
    mooseAssert(_current_elem->dim() == _mesh.dimension(),
                "Below logic only applicable if element and mesh dimension are the same");
    const auto dimensional_component = i % _mesh.dimension();
    auto & d2vel = get_d2(dimensional_component);
    const auto dof_index = vel_dof_indices[i];
    ADReal dof_value = (*_velocity_var->sys().currentSolution())(dof_index);
    if (doVelocityDerivatives())
      dof_value.derivatives().insert(dof_index) = 1;
    const auto scalar_i_component = i / _mesh.dimension();
    for (const auto qp : make_range(_qrule->n_points()))
      d2vel[qp] += dof_value * _scalar_lagrange_fe->get_d2phi()[scalar_i_component][qp];
  }

  // Now that we have the second order spatial derivatives of velocity, we can compute the strong
  // form of the viscous residual for use in our stabilization calculations
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    _viscous_strong_residual[_qp](0) = -_mu[_qp] * _d2u[_qp].tr();
    _viscous_strong_residual[_qp](1) =
        _mesh.dimension() >= 2 ? -_mu[_qp] * _d2v[_qp].tr() : ADReal(0);
    _viscous_strong_residual[_qp](2) =
        _mesh.dimension() == 3 ? -_mu[_qp] * _d2w[_qp].tr() : ADReal(0);
    if (_viscous_form == NS::ViscousForm::Traction)
    {
      _viscous_strong_residual[_qp] -= _mu[_qp] * _d2u[_qp].row(0);
      if (_mesh.dimension() >= 2)
        _viscous_strong_residual[_qp] -= _mu[_qp] * _d2v[_qp].row(1);
      if (_mesh.dimension() == 3)
        _viscous_strong_residual[_qp] -= _mu[_qp] * _d2w[_qp].row(2);
    }
    if (_coord_sys == Moose::COORD_RZ)
      viscousTermRZ();
  }
}

template <typename T>
void
INSADTauMaterialTempl<T>::viscousTermRZ()
{
  // To understand the code immediately below, visit
  // https://en.wikipedia.org/wiki/Del_in_cylindrical_and_spherical_coordinates.
  // The u_r / r^2 term comes from the vector Laplacian. The -du_i/dr * 1/r term comes from
  // the scalar Laplacian. The scalar Laplacian in axisymmetric cylindrical coordinates is
  // equivalent to the Cartesian Laplacian plus a 1/r * du_i/dr term. And of course we are
  // applying a minus sign here because the strong form is -\nabala^2 * \vec{u}
  //
  // Another note: libMesh implements grad(v) as dvi/dxj

  const auto r = _ad_q_point[_qp](_rz_radial_coord);

  if (_viscous_form == NS::ViscousForm::Laplace)
    _viscous_strong_residual[_qp] +=
        // u_r
        // Additional term from vector Laplacian
        ADRealVectorValue(_mu[_qp] * (_velocity[_qp](_rz_radial_coord) / (r * r) -
                                      // Additional term from scalar Laplacian
                                      _grad_velocity[_qp](_rz_radial_coord, _rz_radial_coord) / r),
                          // u_z
                          // Additional term from scalar Laplacian
                          -_mu[_qp] * _grad_velocity[_qp](_rz_axial_coord, _rz_radial_coord) / r,
                          0);
  else
    _viscous_strong_residual[_qp] +=
        ADRealVectorValue(2. * _mu[_qp] *
                              (_velocity[_qp](_rz_radial_coord) / (r * r) -
                               _grad_velocity[_qp](_rz_radial_coord, _rz_radial_coord) / r),
                          -_mu[_qp] / r * (_grad_velocity[_qp](1, 0) + _grad_velocity[_qp](0, 1)),
                          0);
}

template <typename T>
void
INSADTauMaterialTempl<T>::computeQpProperties()
{
  T::computeQpProperties();

  const auto nu = _mu[_qp] / _rho[_qp];
  const auto transient_part = _has_transient ? 4. / (_dt * _dt) : 0.;
  const auto speed = NS::computeSpeed(_velocity[_qp]);
  _tau[_qp] = _alpha / std::sqrt(transient_part + (2. * speed / _hmax) * (2. * speed / _hmax) +
                                 9. * (4. * nu / (_hmax * _hmax)) * (4. * nu / (_hmax * _hmax)));

  _momentum_strong_residual[_qp] =
      _advective_strong_residual[_qp] + _viscous_strong_residual[_qp] + _grad_p[_qp];

  if (_has_transient)
    _momentum_strong_residual[_qp] += _td_strong_residual[_qp];

  if (_has_gravity)
    _momentum_strong_residual[_qp] += _gravity_strong_residual[_qp];

  if (_has_boussinesq)
    _momentum_strong_residual[_qp] += _boussinesq_strong_residual[_qp];

  if (_has_convected_mesh)
    _momentum_strong_residual[_qp] += _convected_mesh_strong_residual[_qp];

  if (_has_coupled_force)
    _momentum_strong_residual[_qp] += _coupled_force_strong_residual[_qp];

  // // Future addition
  // if (_object_tracker->hasMMS())
  //   _momentum_strong_residual[_qp] += _mms_function_strong_residual[_qp];
}
