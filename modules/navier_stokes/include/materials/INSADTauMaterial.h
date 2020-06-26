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

  ADReal _hmax;

  using T::_current_elem;
  using T::_displacements;
  using T::_dt;
  using T::_fe_problem;
  using T::_has_transient;
  using T::_mu;
  using T::_object_tracker;
  using T::_qp;
  using T::_rho;
  using T::_velocity;
};

typedef INSADTauMaterialTempl<INSADMaterial> INSADTauMaterial;
typedef INSADTauMaterialTempl<INSAD3Eqn> INSADStabilized3Eqn;

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
    _tau(this->template declareADProperty<Real>("tau"))
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

  auto && nu = _mu[_qp] / _rho[_qp];
  auto && transient_part = _has_transient ? 4. / (_dt * _dt) : 0.;
  _tau[_qp] = _alpha / std::sqrt(transient_part +
                                 (2. * _velocity[_qp].norm() / _hmax) *
                                     (2. * _velocity[_qp].norm() / _hmax) +
                                 9. * (4. * nu / (_hmax * _hmax)) * (4. * nu / (_hmax * _hmax)));
}
