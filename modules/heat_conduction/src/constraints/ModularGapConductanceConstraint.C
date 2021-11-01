//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ModularGapConductanceConstraint.h"
#include "GapFluxModelBase.h"

registerMooseObject("HeatConductionApp", ModularGapConductanceConstraint);

InputParameters
ModularGapConductanceConstraint::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();
  params.addClassDescription(
      "Computes the residual and Jacobian contributions for the 'Lagrange Multiplier' "
      "implementation of the thermal contact problem. For more information, see the "
      "detailed description here: http://tinyurl.com/gmmhbe9");
  params.addParam<std::vector<UserObjectName>>("gap_flux_models",
                                               "List of GapFluxModel user objects");
  params.addCoupledVar("displacements", "Displacement variables");
  return params;
}

ModularGapConductanceConstraint::ModularGapConductanceConstraint(const InputParameters & parameters)
  : ADMortarConstraint(parameters),
    _gap_flux_model_names(getParam<std::vector<UserObjectName>>("gap_flux_models")),
    _disp_name(parameters.getVecMooseType("displacements")),
    _n_disp(_disp_name.size()),
    _disp_secondary(_n_disp),
    _disp_primary(_n_disp)
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("ModularGapConductanceConstraint relies on use of the global indexing container "
             "in order to make its implementation feasible");
#endif

  for (unsigned int i = 0; i < _n_disp; ++i)
  {
    auto & disp_var = _subproblem.getStandardVariable(_tid, _disp_name[i]);
    _disp_secondary[i] = &disp_var.adSln();
    _disp_primary[i] = &disp_var.adSlnNeighbor();
  }

  for (const auto & name : _gap_flux_model_names)
  {
    const auto & gap_model = getUserObjectByName<GapFluxModelBase>(name);

    // This constraint explicitly calls the gap flux model user objects to
    // obtain contributions to its residuals. It therefore depends on all
    // variables and material properties, that these gap flux models use, to be
    // computed and up to date. To ensure that we collect all variable and
    // material property dependencies of these models and declare them as
    // dependencies of this constraint object. This turns an implicit, hidden
    // dependency into an explicit dependency that MOOSE will automatically fulfill.

    // pass variable dependencies through
    const auto & var_dependencies = gap_model.getMooseVariableDependencies();
    for (const auto & var : var_dependencies)
      addMooseVariableDependency(var);

    // pass material property dependencies through
    const auto & mat_dependencies = gap_model.getMatPropDependencies();
    _material_property_dependencies.insert(mat_dependencies.begin(), mat_dependencies.end());

    // add gap model to list
    _gap_flux_models.push_back(&gap_model);
  }
}

ADReal
ModularGapConductanceConstraint::computeQpResidual(Moose::MortarType mortar_type)
{
  switch (mortar_type)
  {
    case Moose::MortarType::Primary:
      return _lambda[_qp] * _test_primary[_i][_qp];
    case Moose::MortarType::Secondary:
      return -_lambda[_qp] * _test_secondary[_i][_qp];
    case Moose::MortarType::Lower:
    {
      // we are creating an AD version of phys points primary and secondary here...
      ADRealVectorValue ad_phys_points_primary = _phys_points_primary[_qp];
      ADRealVectorValue ad_phys_points_secondary = _phys_points_secondary[_qp];

      // ...which uses the derivative vector of the primary and secondary displacements as
      // an approximation of the true phys points derivatives when the mesh is displacing
      if (_displaced)
        for (unsigned int i = 0; i < _n_disp; ++i)
        {
          ad_phys_points_primary(i).derivatives() = (*_disp_primary[i])[_qp].derivatives();
          ad_phys_points_secondary(i).derivatives() = (*_disp_secondary[i])[_qp].derivatives();
        }

      // compute an ADReal gap width to pass to each gap flux model
      const auto gap_width = (ad_phys_points_primary - ad_phys_points_secondary) * _normals[_qp];

      // sum up all flux contributions from all supplied gap flux models
      ADReal flux = 0.0;
      for (const auto & model : _gap_flux_models)
        flux += model->computeFlux(gap_width, _qp);

      // the Lagrange multiplier _is_ the gap flux
      return (_lambda[_qp] - flux) * _test[_i][_qp];
    }

    default:
      return 0;
  }
}
