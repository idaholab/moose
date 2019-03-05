//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "INSADTauMaterial.h"
#include "NonlinearSystemBase.h"

defineADValidParams(
    INSADTauMaterial,
    INSADMaterial,
    params.addClassDescription(
        "This is the material class used to compute the stabilization parameter tau.");
    params.addRequiredCoupledVar("velocity", "The velocity");
    params.addParam<MaterialPropertyName>("mu_name", "mu", "The name of the dynamic viscosity");
    params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");

    params.addParam<Real>("alpha", 1., "Multiplicative factor on the stabilization parameter tau.");
    params.addParam<bool>("transient_term",
                          false,
                          "Whether there should be a transient term in the momentum residuals."););

template <ComputeStage compute_stage>
INSADTauMaterial<compute_stage>::INSADTauMaterial(const InputParameters & parameters)
  : INSADMaterial<compute_stage>(parameters),
    _alpha(adGetParam<Real>("alpha")),
    _tau(adDeclareADProperty<Real>("tau"))
{
}

template <ComputeStage compute_stage>
void
INSADTauMaterial<compute_stage>::computeHMax()
{
  _hmax = _current_elem->hmax();
}

template <>
void
INSADTauMaterial<JACOBIAN>::computeHMax()
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

template <ComputeStage compute_stage>
void
INSADTauMaterial<compute_stage>::computeProperties()
{
  computeHMax();

  Material::computeProperties();
}

template <ComputeStage compute_stage>
void
INSADTauMaterial<compute_stage>::computeQpProperties()
{
  auto && nu = _mu[_qp] / _rho[_qp];
  auto && transient_part = _transient_term ? 4. / (_dt * _dt) : 0.;
  _tau[_qp] = _alpha / std::sqrt(transient_part +
                                 (2. * _velocity.norm() / _hmax) * (2. * _velocity.norm() / _hmax) +
                                 9. * (4. * nu / (_hmax * _hmax)) * (4. * nu / (_hmax * _hmax)));
}
