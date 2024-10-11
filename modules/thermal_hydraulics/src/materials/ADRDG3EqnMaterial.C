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
#include "THMIndicesVACE.h"
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
  _U_vars.resize(THMVACE1D::N_FLUX_INPUTS);
  _U_vars[THMVACE1D::RHOA] = _rhoA_var;
  _U_vars[THMVACE1D::RHOUA] = _rhouA_var;
  _U_vars[THMVACE1D::RHOEA] = _rhoEA_var;
  _U_vars[THMVACE1D::AREA] = _A_var;
}

void
ADRDG3EqnMaterial::computeQpProperties()
{
  if (_scheme == None)
  {
    const auto A_ratio = _A_linear[_qp] / _A_avg[_qp];
    _rhoA[_qp] = _rhoA_avg[_qp] * A_ratio;
    _rhouA[_qp] = _rhouA_avg[_qp] * A_ratio;
    _rhoEA[_qp] = _rhoEA_avg[_qp] * A_ratio;
  }
  else
  {
    // compute primitive variables from the cell-average solution
    std::vector<ADReal> U_avg(THMVACE1D::N_FLUX_INPUTS, 0.0);
    U_avg[THMVACE1D::RHOA] = _rhoA_avg[_qp];
    U_avg[THMVACE1D::RHOUA] = _rhouA_avg[_qp];
    U_avg[THMVACE1D::RHOEA] = _rhoEA_avg[_qp];
    U_avg[THMVACE1D::AREA] = _A_avg[_qp];
    auto W = FlowModel1PhaseUtils::computePrimitiveSolutionVector<true>(U_avg, _fp);

    // compute and apply slopes to primitive variables
    const auto slopes = getElementSlopes(_current_elem);
    const auto delta_x = (_q_point[_qp] - _current_elem->vertex_average()) * _dir[_qp];
    for (unsigned int m = 0; m < THMVACE1D::N_PRIM_VARS; m++)
      W[m] = W[m] + slopes[m] * delta_x;

    // compute reconstructed conservative variables
    const auto U =
        FlowModel1PhaseUtils::computeConservativeSolutionVector<true>(W, _A_linear[_qp], _fp);
    _rhoA[_qp] = U[THMVACE1D::RHOA];
    _rhouA[_qp] = U[THMVACE1D::RHOUA];
    _rhoEA[_qp] = U[THMVACE1D::RHOEA];
  }
}

std::vector<ADReal>
ADRDG3EqnMaterial::computeElementPrimitiveVariables(const Elem * elem) const
{
  const auto U =
      FlowModel1PhaseUtils::getElementalSolutionVector<true>(elem, _U_vars, _is_implicit);
  return FlowModel1PhaseUtils::computePrimitiveSolutionVector<true>(U, _fp);
}
