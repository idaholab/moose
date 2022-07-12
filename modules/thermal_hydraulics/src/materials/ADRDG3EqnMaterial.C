//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADRDG3EqnMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "THMIndices3Eqn.h"
#include "FlowModel1PhaseUtils.h"

registerMooseObject("ThermalHydraulicsApp", ADRDG3EqnMaterial);

InputParameters
ADRDG3EqnMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params += SlopeReconstruction1DInterface<true>::validParams();

  params.addClassDescription(
      "Reconstructed solution values for the 1-D, 1-phase, variable-area Euler equations");

  params.addRequiredCoupledVar("A_elem", "Cross-sectional area, elemental");
  params.addRequiredCoupledVar("A_linear", "Cross-sectional area, linear");
  params.addRequiredCoupledVar("rhoA", "Conserved variable: rho*A");
  params.addRequiredCoupledVar("rhouA", "Conserved variable: rho*u*A");
  params.addRequiredCoupledVar("rhoEA", "Conserved variable: rho*E*A");

  params.addRequiredParam<MaterialPropertyName>("direction",
                                                "Flow channel direction material property name");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of fluid properties user object");

  return params;
}

ADRDG3EqnMaterial::ADRDG3EqnMaterial(const InputParameters & parameters)
  : Material(parameters),
    SlopeReconstruction1DInterface<true>(this),

    _A_avg(adCoupledValue("A_elem")),
    _A_linear(adCoupledValue("A_linear")),
    _rhoA_avg(adCoupledValue("rhoA")),
    _rhouA_avg(adCoupledValue("rhouA")),
    _rhoEA_avg(adCoupledValue("rhoEA")),

    _A_var(getVar("A_elem", 0)),
    _rhoA_var(getVar("rhoA", 0)),
    _rhouA_var(getVar("rhouA", 0)),
    _rhoEA_var(getVar("rhoEA", 0)),

    _dir(getMaterialProperty<RealVectorValue>("direction")),

    _rhoA(declareADProperty<Real>("rhoA")),
    _rhouA(declareADProperty<Real>("rhouA")),
    _rhoEA(declareADProperty<Real>("rhoEA")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

void
ADRDG3EqnMaterial::computeQpProperties()
{
  // compute primitive variables from the cell-average solution
  std::vector<ADReal> U_avg(THM3Eqn::N_CONS_VAR, 0.0);
  U_avg[THM3Eqn::CONS_VAR_RHOA] = _rhoA_avg[_qp];
  U_avg[THM3Eqn::CONS_VAR_RHOUA] = _rhouA_avg[_qp];
  U_avg[THM3Eqn::CONS_VAR_RHOEA] = _rhoEA_avg[_qp];
  U_avg[THM3Eqn::CONS_VAR_AREA] = _A_avg[_qp];
  auto W = FlowModel1PhaseUtils::computePrimitiveSolutionVector<true>(U_avg, _fp);

  // compute and apply slopes to primitive variables
  const auto slopes = getElementSlopes(_current_elem);
  const auto delta_x = (_q_point[_qp] - _current_elem->vertex_average()) * _dir[_qp];
  for (unsigned int m = 0; m < THM3Eqn::N_PRIM_VAR; m++)
    W[m] = W[m] + slopes[m] * delta_x;

  // compute reconstructed conservative variables
  const auto U =
      FlowModel1PhaseUtils::computeConservativeSolutionVector<true>(W, _A_linear[_qp], _fp);
  _rhoA[_qp] = U[THM3Eqn::CONS_VAR_RHOA];
  _rhouA[_qp] = U[THM3Eqn::CONS_VAR_RHOUA];
  _rhoEA[_qp] = U[THM3Eqn::CONS_VAR_RHOEA];
}

std::vector<ADReal>
ADRDG3EqnMaterial::computeElementPrimitiveVariables(const Elem * elem) const
{
  // get the cell-average conserved variables
  ADReal A, rhoA, rhouA, rhoEA;
  if (_is_implicit)
  {
    A = _A_var->getElementalValue(elem);
    rhoA = _rhoA_var->getElementalValue(elem);
    rhouA = _rhouA_var->getElementalValue(elem);
    rhoEA = _rhoEA_var->getElementalValue(elem);

#ifdef MOOSE_GLOBAL_AD_INDEXING
    std::vector<dof_id_type> dof_indices;

    _rhoA_var->dofMap().dof_indices(elem, dof_indices, _rhoA_var->number());
    Moose::derivInsert(rhoA.derivatives(), dof_indices[0], 1.0);

    _rhouA_var->dofMap().dof_indices(elem, dof_indices, _rhouA_var->number());
    Moose::derivInsert(rhouA.derivatives(), dof_indices[0], 1.0);

    _rhoEA_var->dofMap().dof_indices(elem, dof_indices, _rhoEA_var->number());
    Moose::derivInsert(rhoEA.derivatives(), dof_indices[0], 1.0);
#else
    mooseError("Only global AD indexing is supported.");
#endif
  }
  else
  {
    A = _A_var->getElementalValueOld(elem);
    rhoA = _rhoA_var->getElementalValueOld(elem);
    rhouA = _rhouA_var->getElementalValueOld(elem);
    rhoEA = _rhoEA_var->getElementalValueOld(elem);
  }

  // compute primitive variables
  std::vector<ADReal> U(THM3Eqn::N_CONS_VAR, 0.0);
  U[THM3Eqn::CONS_VAR_RHOA] = rhoA;
  U[THM3Eqn::CONS_VAR_RHOUA] = rhouA;
  U[THM3Eqn::CONS_VAR_RHOEA] = rhoEA;
  U[THM3Eqn::CONS_VAR_AREA] = A;
  return FlowModel1PhaseUtils::computePrimitiveSolutionVector<true>(U, _fp);
}
