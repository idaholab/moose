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

#include "libmesh/elem.h"

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
  void computeHMax();

  const Real _alpha;
  ADMaterialProperty<Real> & _tau;

  /// The strong residual of the momentum equation
  ADMaterialProperty<RealVectorValue> & _momentum_strong_residual;

  ADReal _hmax;

  using T::_advective_strong_residual;
  using T::_boussinesq_strong_residual;
  using T::_coord_sys;
  using T::_coupled_force_strong_residual;
  using T::_current_elem;
  using T::_displacements;
  using T::_dt;
  using T::_fe_problem;
  using T::_grad_p;
  using T::_gravity_strong_residual;
  using T::_has_boussinesq;
  using T::_has_coupled_force;
  using T::_has_gravity;
  using T::_has_transient;
  using T::_mu;
  using T::_object_tracker;
  using T::_qp;
  using T::_rho;
  using T::_td_strong_residual;
  using T::_velocity;
  using T::_viscous_strong_residual;
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
    _momentum_strong_residual(
        this->template declareADProperty<RealVectorValue>("momentum_strong_residual"))
{
}

template <typename T>
void
INSADTauMaterialTempl<T>::computeHMax()
{
  if (!_displacements.size())
  {
    _hmax = _current_elem->hmax();
    return;
  }

  _hmax = 0;

  for (unsigned int n_outer = 0; n_outer < _current_elem->n_vertices(); n_outer++)
    for (unsigned int n_inner = n_outer + 1; n_inner < _current_elem->n_vertices(); n_inner++)
    {
      VectorValue<DualReal> diff = (_current_elem->point(n_outer) - _current_elem->point(n_inner));
      unsigned dimension = 0;
      for (const auto & disp_num : _displacements)
      {
        diff(dimension)
            .derivatives()[disp_num * _fe_problem.getNonlinearSystemBase().getMaxVarNDofsPerElem() +
                           n_outer] = 1.;
        diff(dimension++)
            .derivatives()[disp_num * _fe_problem.getNonlinearSystemBase().getMaxVarNDofsPerElem() +
                           n_inner] = -1.;
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

  T::computeProperties();
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

  _momentum_strong_residual[_qp] = _advective_strong_residual[_qp] + _grad_p[_qp];

  // Since we can't current compute vector Laplacians we only have strong residual contributions
  // from the viscous term in the RZ coordinate system
  if (_coord_sys == Moose::COORD_RZ)
    _momentum_strong_residual[_qp] += _viscous_strong_residual[_qp];

  if (_has_transient)
    _momentum_strong_residual[_qp] += _td_strong_residual[_qp];

  if (_has_gravity)
    _momentum_strong_residual[_qp] += _gravity_strong_residual[_qp];

  if (_has_boussinesq)
    _momentum_strong_residual[_qp] += _boussinesq_strong_residual[_qp];

  if (_has_coupled_force)
    _momentum_strong_residual[_qp] += _coupled_force_strong_residual[_qp];

  // // Future addition
  // if (_object_tracker->hasMMS())
  //   _momentum_strong_residual[_qp] += _mms_function_strong_residual[_qp];
}
