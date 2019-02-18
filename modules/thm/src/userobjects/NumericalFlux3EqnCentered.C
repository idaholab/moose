#include "NumericalFlux3EqnCentered.h"
#include "THMIndices3Eqn.h"
#include "Numerics.h"

registerMooseObject("THMApp", NumericalFlux3EqnCentered);

template <>
InputParameters
validParams<NumericalFlux3EqnCentered>()
{
  InputParameters params = validParams<NumericalFlux3EqnBase>();

  params.addClassDescription(
      "Computes internal side flux for the 1-D, 1-phase, variable-area Euler equations using a "
      "centered average of the left and right side fluxes");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");

  return params;
}

NumericalFlux3EqnCentered::NumericalFlux3EqnCentered(const InputParameters & parameters)
  : NumericalFlux3EqnBase(parameters),

    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

void
NumericalFlux3EqnCentered::calcFlux(const std::vector<Real> & U1,
                                    const std::vector<Real> & U2,
                                    const RealVectorValue & /*normal*/,
                                    std::vector<Real> & flux) const
{
  const std::vector<Real> flux1 = computeFlux(U1);
  const std::vector<Real> flux2 = computeFlux(U2);

  flux.resize(THM3Eqn::N_EQ);
  for (unsigned int i = 0; i < THM3Eqn::N_EQ; i++)
    flux[i] = 0.5 * (flux1[i] + flux2[i]);
}

std::vector<Real>
NumericalFlux3EqnCentered::computeFlux(const std::vector<Real> & U) const
{
  const Real rhoA = U[THM3Eqn::CONS_VAR_RHOA];
  const Real rhouA = U[THM3Eqn::CONS_VAR_RHOUA];
  const Real rhoEA = U[THM3Eqn::CONS_VAR_RHOEA];
  const Real A = U[THM3Eqn::CONS_VAR_AREA];

  const Real rho = rhoA / A;
  const Real vel = rhouA / rhoA;
  const Real v = 1.0 / rho;
  const Real E = rhoEA / rhoA;
  const Real e = E - 0.5 * vel * vel;
  const Real p = _fp.p_from_v_e(v, e);
  const Real H = E + p / rho;

  std::vector<Real> flux(THM3Eqn::N_EQ, 0.0);
  flux[THM3Eqn::EQ_MASS] = rhouA;
  flux[THM3Eqn::EQ_MOMENTUM] = (rho * vel * vel + p) * A;
  flux[THM3Eqn::EQ_ENERGY] = rho * vel * H * A;

  return flux;
}

void
NumericalFlux3EqnCentered::calcJacobian(const std::vector<Real> & U1,
                                        const std::vector<Real> & U2,
                                        const RealVectorValue & /*normal*/,
                                        DenseMatrix<Real> & jac1,
                                        DenseMatrix<Real> & jac2) const
{
  jac1 = computeJacobian(U1);
  jac2 = computeJacobian(U2);

  jac1 *= 0.5;
  jac2 *= 0.5;
}

DenseMatrix<Real>
NumericalFlux3EqnCentered::computeJacobian(const std::vector<Real> & U) const
{
  const Real rhoA = U[THM3Eqn::CONS_VAR_RHOA];
  const Real rhouA = U[THM3Eqn::CONS_VAR_RHOUA];
  const Real rhoEA = U[THM3Eqn::CONS_VAR_RHOEA];
  const Real A = U[THM3Eqn::CONS_VAR_AREA];

  const Real v = A / rhoA;
  const Real dv_drhoA = THM::dv_darhoA(A, rhoA);

  const Real vel = rhouA / rhoA;

  const Real e = rhoEA / rhoA - 0.5 * rhouA * rhouA / (rhoA * rhoA);
  const Real de_drhoA = THM::de_darhoA(rhoA, rhouA, rhoEA);
  const Real de_drhouA = THM::de_darhouA(rhoA, rhouA);
  const Real de_drhoEA = THM::de_darhoEA(rhoA);

  Real p, dp_dv, dp_de;
  _fp.p_from_v_e(v, e, p, dp_dv, dp_de);
  const Real dp_drhoA = dp_dv * dv_drhoA + dp_de * de_drhoA;
  const Real dp_drhouA = dp_de * de_drhouA;
  const Real dp_drhoEA = dp_de * de_drhoEA;

  DenseMatrix<Real> jac(THM3Eqn::N_EQ, THM3Eqn::N_EQ);

  jac(THM3Eqn::EQ_MASS, THM3Eqn::EQ_MOMENTUM) = 1.0;

  jac(THM3Eqn::EQ_MOMENTUM, THM3Eqn::EQ_MASS) = -vel * vel + dp_drhoA * A;
  jac(THM3Eqn::EQ_MOMENTUM, THM3Eqn::EQ_MOMENTUM) = 2.0 * vel + dp_drhouA * A;
  jac(THM3Eqn::EQ_MOMENTUM, THM3Eqn::EQ_ENERGY) = dp_drhoEA * A;

  jac(THM3Eqn::EQ_ENERGY, THM3Eqn::EQ_MASS) = -vel / rhoA * (rhoEA + p * A) + vel * dp_drhoA * A;
  jac(THM3Eqn::EQ_ENERGY, THM3Eqn::EQ_MOMENTUM) = (rhoEA + p * A) / rhoA + vel * dp_drhouA * A;
  jac(THM3Eqn::EQ_ENERGY, THM3Eqn::EQ_ENERGY) = vel * (1.0 + dp_drhoEA * A);

  return jac;
}
