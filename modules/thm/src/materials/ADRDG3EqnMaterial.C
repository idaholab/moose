#include "ADRDG3EqnMaterial.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("THMApp", ADRDG3EqnMaterial);

InputParameters
ADRDG3EqnMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params += SlopeReconstruction1DInterface::validParams();

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
    SlopeReconstruction1DInterface(this),

    _A_avg(adCoupledValue("A_elem")),
    _A_linear(adCoupledValue("A_linear")),
    _rhoA_avg(adCoupledValue("rhoA")),
    _rhouA_avg(adCoupledValue("rhouA")),
    _rhoEA_avg(adCoupledValue("rhoEA")),

    _A_var(getVar("A_elem", 0)),
    _rhoA_var(getVar("rhoA", 0)),
    _rhouA_var(getVar("rhouA", 0)),
    _rhoEA_var(getVar("rhoEA", 0)),

    _dir(getADMaterialProperty<RealVectorValue>("direction")),

    _rhoA(declareADProperty<Real>("rhoA")),
    _rhouA(declareADProperty<Real>("rhouA")),
    _rhoEA(declareADProperty<Real>("rhoEA")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

void
ADRDG3EqnMaterial::computeQpProperties()
{
  // Get the limited slopes of the primitive variables: {p, u, T}.
  const auto slopes = getElementSlopes(_current_elem);
  const ADReal p_slope = slopes[PRESSURE];
  const ADReal vel_slope = slopes[VELOCITY];
  const ADReal T_slope = slopes[TEMPERATURE];

  // compute primitive variables from the cell-average solution
  const ADReal rho_avg = _rhoA_avg[_qp] / _A_avg[_qp];
  const ADReal vel_avg = _rhouA_avg[_qp] / _rhoA_avg[_qp];
  const ADReal v_avg = 1.0 / rho_avg;
  const ADReal e_avg = _rhoEA_avg[_qp] / _rhoA_avg[_qp] - 0.5 * vel_avg * vel_avg;
  const ADReal p_avg = _fp.p_from_v_e(v_avg, e_avg);
  const ADReal T_avg = _fp.T_from_v_e(v_avg, e_avg);

  // apply slopes to primitive variables
  const ADReal delta_x = (_q_point[_qp] - _current_elem->centroid()) * _dir[_qp];
  const ADReal p = p_avg + p_slope * delta_x;
  const ADReal vel = vel_avg + vel_slope * delta_x;
  const ADReal T = T_avg + T_slope * delta_x;

  // compute reconstructed conserved variables
  const ADReal rho = _fp.rho_from_p_T(p, T);
  const ADReal e = _fp.e_from_p_rho(p, rho);
  const ADReal E = e + 0.5 * vel * vel;

  _rhoA[_qp] = rho * _A_linear[_qp];
  _rhouA[_qp] = _rhoA[_qp] * vel;
  _rhoEA[_qp] = _rhoA[_qp] * E;
}

std::vector<Real>
ADRDG3EqnMaterial::computeElementPrimitiveVariables(const Elem * elem) const
{
  // get the cell-average conserved variables
  Real A, rhoA, rhouA, rhoEA;
  if (_is_implicit)
  {
    A = _A_var->getElementalValue(elem);
    rhoA = _rhoA_var->getElementalValue(elem);
    rhouA = _rhouA_var->getElementalValue(elem);
    rhoEA = _rhoEA_var->getElementalValue(elem);
  }
  else
  {
    A = _A_var->getElementalValueOld(elem);
    rhoA = _rhoA_var->getElementalValueOld(elem);
    rhouA = _rhouA_var->getElementalValueOld(elem);
    rhoEA = _rhoEA_var->getElementalValueOld(elem);
  }

  // compute primitive variables

  const Real rho = rhoA / A;
  const Real vel = rhouA / rhoA;
  const Real v = 1.0 / rho;
  const Real e = rhoEA / rhoA - 0.5 * vel * vel;

  std::vector<Real> W(_n_slopes);
  W[PRESSURE] = _fp.p_from_v_e(v, e);
  W[VELOCITY] = vel;
  W[TEMPERATURE] = _fp.T_from_v_e(v, e);

  return W;
}
