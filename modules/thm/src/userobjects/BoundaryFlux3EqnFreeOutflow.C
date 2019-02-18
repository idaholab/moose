#include "BoundaryFlux3EqnFreeOutflow.h"
#include "THMIndices3Eqn.h"
#include "Numerics.h"

registerMooseObject("THMApp", BoundaryFlux3EqnFreeOutflow);

template <>
InputParameters
validParams<BoundaryFlux3EqnFreeOutflow>()
{
  InputParameters params = validParams<BoundaryFluxBase>();

  params.addClassDescription("Computes the outflow boundary flux directly for the 1-D, 1-phase, "
                             "variable-area Euler equations");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of fluid properties user object");

  return params;
}

BoundaryFlux3EqnFreeOutflow::BoundaryFlux3EqnFreeOutflow(const InputParameters & parameters)
  : BoundaryFluxBase(parameters),

    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

void
BoundaryFlux3EqnFreeOutflow::calcFlux(unsigned int /*iside*/,
                                      dof_id_type /*ielem*/,
                                      const std::vector<Real> & U1,
                                      const RealVectorValue & /*normal*/,
                                      std::vector<Real> & flux) const
{
  // extract the solution and area
  const Real rhoA1 = U1[THM3Eqn::CONS_VAR_RHOA];
  const Real rhouA1 = U1[THM3Eqn::CONS_VAR_RHOUA];
  const Real rhoEA1 = U1[THM3Eqn::CONS_VAR_RHOEA];
  const Real A1 = U1[THM3Eqn::CONS_VAR_AREA];

  const Real rho1 = rhoA1 / A1;
  const Real vel1 = rhouA1 / rhoA1;
  const Real v1 = 1.0 / rho1;
  const Real e1 = rhoEA1 / rhoA1 - 0.5 * vel1 * vel1;
  const Real p1 = _fp.p_from_v_e(v1, e1);

  flux.resize(THM3Eqn::N_EQ);
  flux[THM3Eqn::EQ_MASS] = vel1 * rhoA1;
  flux[THM3Eqn::EQ_MOMENTUM] = (vel1 * rhouA1 + p1 * A1);
  flux[THM3Eqn::EQ_ENERGY] = vel1 * (rhoEA1 + p1 * A1);
}

void
BoundaryFlux3EqnFreeOutflow::calcJacobian(unsigned int /*iside*/,
                                          dof_id_type /*ielem*/,
                                          const std::vector<Real> & U1,
                                          const RealVectorValue & /*normal*/,
                                          DenseMatrix<Real> & jac1) const
{
  const Real rhoA = U1[THM3Eqn::CONS_VAR_RHOA];
  const Real rhouA = U1[THM3Eqn::CONS_VAR_RHOUA];
  const Real rhoEA = U1[THM3Eqn::CONS_VAR_RHOEA];
  const Real A = U1[THM3Eqn::CONS_VAR_AREA];

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

  jac1.resize(THM3Eqn::N_EQ, THM3Eqn::N_EQ);

  jac1(THM3Eqn::EQ_MASS, THM3Eqn::EQ_MOMENTUM) = 1.0;

  jac1(THM3Eqn::EQ_MOMENTUM, THM3Eqn::EQ_MASS) = -vel * vel + dp_drhoA * A;
  jac1(THM3Eqn::EQ_MOMENTUM, THM3Eqn::EQ_MOMENTUM) = 2.0 * vel + dp_drhouA * A;
  jac1(THM3Eqn::EQ_MOMENTUM, THM3Eqn::EQ_ENERGY) = dp_drhoEA * A;

  jac1(THM3Eqn::EQ_ENERGY, THM3Eqn::EQ_MASS) = -vel / rhoA * (rhoEA + p * A) + vel * dp_drhoA * A;
  jac1(THM3Eqn::EQ_ENERGY, THM3Eqn::EQ_MOMENTUM) = (rhoEA + p * A) / rhoA + vel * dp_drhouA * A;
  jac1(THM3Eqn::EQ_ENERGY, THM3Eqn::EQ_ENERGY) = vel * (1.0 + dp_drhoEA * A);
}
