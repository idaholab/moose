//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCheckGlobalToDerivativeMap.h"
#include "ADUtils.h"
#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"
#include "libmesh/system.h"
#include "libmesh/dof_map.h"

registerMooseObject("MooseTestApp", ADCheckGlobalToDerivativeMap);

InputParameters
ADCheckGlobalToDerivativeMap::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("u", "A variable u");
  params.addRequiredCoupledVar("v", "A variable v");
  params.addRequiredParam<MaterialPropertyName>("mat_prop",
                                                "Name of the ad property this material defines");
  return params;
}

ADCheckGlobalToDerivativeMap::ADCheckGlobalToDerivativeMap(const InputParameters & parameters)
  : Material(parameters),
    _mat_prop(declareADProperty<Real>(getParam<MaterialPropertyName>("mat_prop"))),
    _u(adCoupledValue("u")),
    _v(adCoupledValue("v")),
    _u_var(*getVar("u", 0)),
    _v_var(*getVar("v", 0))
{
}

void
ADCheckGlobalToDerivativeMap::computeProperties()
{
  Material::computeProperties();

// Avoid unused variable warnings
#ifndef NDEBUG
  if (_fe_problem.currentlyComputingJacobian())
  {
    const auto & moose_nl_system = _fe_problem.getNonlinearSystemBase();

    const auto global_index_to_deriv_map = Moose::globalDofIndexToDerivative(
        _mat_prop, moose_nl_system, Moose::ElementType::Element, _tid);

    const DofMap & dof_map = moose_nl_system.system().get_dof_map();

    std::vector<dof_id_type> u_dof_indices, v_dof_indices;
    dof_map.dof_indices(_current_elem, u_dof_indices, _u_var.number());
    dof_map.dof_indices(_current_elem, v_dof_indices, _v_var.number());

    const auto nqp = _qrule->n_points();
    mooseAssert(global_index_to_deriv_map.size() == nqp,
                "The map should have the same size as the number of quadrature points");

    auto max_dofs_per_elem = moose_nl_system.getMaxVarNDofsPerElem();

    const auto u_offset = Moose::adOffset(_u_var.number(), max_dofs_per_elem);
    const auto v_offset = Moose::adOffset(_v_var.number(), max_dofs_per_elem);

    for (MooseIndex(nqp) qp = 0; qp < nqp; ++qp)
    {
      const auto & map_at_qp = global_index_to_deriv_map[qp];
      const ADReal & qp_ad_real = _mat_prop[qp];

      for (MooseIndex(u_dof_indices) u_local_index = 0; u_local_index < u_dof_indices.size();
           ++u_local_index)
      {
        auto it = map_at_qp.find(u_dof_indices[u_local_index]);
        mooseAssert(it != map_at_qp.end(), "The global index key was not found!");

        mooseAssert(MooseUtils::absoluteFuzzyEqual(
                        it->second, qp_ad_real.derivatives()[u_offset + u_local_index]),
                    "The derivative values don't match!");
      }

      for (MooseIndex(v_dof_indices) v_local_index = 0; v_local_index < v_dof_indices.size();
           ++v_local_index)
      {
        auto it = map_at_qp.find(v_dof_indices[v_local_index]);
        mooseAssert(it != map_at_qp.end(), "The global index key was not found!");

        mooseAssert(MooseUtils::absoluteFuzzyEqual(
                        it->second, qp_ad_real.derivatives()[v_offset + v_local_index]),
                    "The derivative values don't match!");
      }
    }
  }
#endif
}

void
ADCheckGlobalToDerivativeMap::computeQpProperties()
{
  _mat_prop[_qp] = 1. + std::pow(_u[_qp], 2) * std::pow(_v[_qp], 3);
}
