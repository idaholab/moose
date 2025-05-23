//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SlopeReconstructionGasMixMaterial.h"
#include "THMIndicesGasMix.h"
#include "FlowModelGasMixUtils.h"
#include "THMNames.h"

registerMooseObject("ThermalHydraulicsApp", SlopeReconstructionGasMixMaterial);

InputParameters
SlopeReconstructionGasMixMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params += SlopeReconstruction1DInterface<true>::validParams();

  params.addClassDescription("Computes reconstructed solution values for FlowModelGasMix.");

  params.addRequiredCoupledVar("A_elem", "Cross-sectional area, elemental");
  params.addRequiredCoupledVar("A_linear", "Cross-sectional area, linear");
  params.addRequiredCoupledVar("xirhoA", "Conserved variable xi*rho*A");
  params.addRequiredCoupledVar("rhoA", "Conserved variable rho*A");
  params.addRequiredCoupledVar("rhouA", "Conserved variable rho*u*A");
  params.addRequiredCoupledVar("rhoEA", "Conserved variable rho*E*A");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The VaporMixtureFluidProperties object");

  return params;
}

SlopeReconstructionGasMixMaterial::SlopeReconstructionGasMixMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    SlopeReconstruction1DInterface<true>(this),

    _A_avg(adCoupledValue("A_elem")),
    _A_linear(adCoupledValue("A_linear")),
    _xirhoA_avg(adCoupledValue("xirhoA")),
    _rhoA_avg(adCoupledValue("rhoA")),
    _rhouA_avg(adCoupledValue("rhouA")),
    _rhoEA_avg(adCoupledValue("rhoEA")),

    _A_var(getVar("A_elem", 0)),
    _xirhoA_var(getVar("xirhoA", 0)),
    _rhoA_var(getVar("rhoA", 0)),
    _rhouA_var(getVar("rhouA", 0)),
    _rhoEA_var(getVar("rhoEA", 0)),

    _dir(getMaterialProperty<RealVectorValue>(THM::DIRECTION)),

    _xirhoA(declareADProperty<Real>(THM::XIRHOA)),
    _rhoA(declareADProperty<Real>(THM::RHOA)),
    _rhouA(declareADProperty<Real>(THM::RHOUA)),
    _rhoEA(declareADProperty<Real>(THM::RHOEA)),

    _fp(getUserObject<VaporMixtureFluidProperties>("fluid_properties"))
{
  _U_vars.resize(THMGasMix1D::N_FLUX_INPUTS);
  _U_vars[THMGasMix1D::XIRHOA] = _xirhoA_var;
  _U_vars[THMGasMix1D::RHOA] = _rhoA_var;
  _U_vars[THMGasMix1D::RHOUA] = _rhouA_var;
  _U_vars[THMGasMix1D::RHOEA] = _rhoEA_var;
  _U_vars[THMGasMix1D::AREA] = _A_var;
}

void
SlopeReconstructionGasMixMaterial::computeQpProperties()
{
  if (_scheme == None)
  {
    const auto A_ratio = _A_linear[_qp] / _A_avg[_qp];
    _xirhoA[_qp] = _xirhoA_avg[_qp] * A_ratio;
    _rhoA[_qp] = _rhoA_avg[_qp] * A_ratio;
    _rhouA[_qp] = _rhouA_avg[_qp] * A_ratio;
    _rhoEA[_qp] = _rhoEA_avg[_qp] * A_ratio;
  }
  else
  {
    // compute primitive variables from the cell-average solution
    std::vector<ADReal> U_avg(THMGasMix1D::N_FLUX_INPUTS, 0.0);
    U_avg[THMGasMix1D::XIRHOA] = _xirhoA_avg[_qp];
    U_avg[THMGasMix1D::RHOA] = _rhoA_avg[_qp];
    U_avg[THMGasMix1D::RHOUA] = _rhouA_avg[_qp];
    U_avg[THMGasMix1D::RHOEA] = _rhoEA_avg[_qp];
    U_avg[THMGasMix1D::AREA] = _A_avg[_qp];
    auto W = FlowModelGasMixUtils::computePrimitiveSolution<true>(U_avg, _fp);

    // compute and apply slopes to primitive variables
    const auto slopes = getElementSlopes(_current_elem);
    const auto delta_x = (_q_point[_qp] - _current_elem->vertex_average()) * _dir[_qp];
    for (unsigned int m = 0; m < THMGasMix1D::N_PRIM_VARS; m++)
      W[m] = W[m] + slopes[m] * delta_x;

    // compute reconstructed conservative variables
    const auto U = FlowModelGasMixUtils::computeConservativeSolution<true>(W, _A_linear[_qp], _fp);
    _xirhoA[_qp] = U[THMGasMix1D::XIRHOA];
    _rhoA[_qp] = U[THMGasMix1D::RHOA];
    _rhouA[_qp] = U[THMGasMix1D::RHOUA];
    _rhoEA[_qp] = U[THMGasMix1D::RHOEA];
  }
}

std::vector<ADReal>
SlopeReconstructionGasMixMaterial::computeElementPrimitiveVariables(const Elem * elem) const
{
  const auto U =
      FlowModelGasMixUtils::getElementalSolutionVector<true>(elem, _U_vars, _is_implicit);
  return FlowModelGasMixUtils::computePrimitiveSolution<true>(U, _fp);
}
