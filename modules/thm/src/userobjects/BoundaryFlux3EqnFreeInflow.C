#include "BoundaryFlux3EqnFreeInflow.h"
#include "THMIndices3Eqn.h"

registerMooseObject("THMApp", BoundaryFlux3EqnFreeInflow);

template <>
InputParameters
validParams<BoundaryFlux3EqnFreeInflow>()
{
  InputParameters params = validParams<BoundaryFluxBase>();

  params.addClassDescription("Computes the inflow boundary flux directly for the 1-D, 1-phase, "
                             "variable-area Euler equations");

  params.addRequiredParam<Real>("rho_infinity", "Far-stream density value");
  params.addRequiredParam<Real>("vel_infinity", "Far-stream velocity value");
  params.addRequiredParam<Real>("p_infinity", "Far-stream pressure value");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of fluid properties user object");

  return params;
}

BoundaryFlux3EqnFreeInflow::BoundaryFlux3EqnFreeInflow(const InputParameters & parameters)
  : BoundaryFluxBase(parameters),

    _rho_inf(getParam<Real>("rho_infinity")),
    _vel_inf(getParam<Real>("vel_infinity")),
    _p_inf(getParam<Real>("p_infinity")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

void
BoundaryFlux3EqnFreeInflow::calcFlux(unsigned int /*iside*/,
                                     dof_id_type /*ielem*/,
                                     const std::vector<Real> & U1,
                                     const RealVectorValue & /*normal*/,
                                     std::vector<Real> & flux) const
{
  const Real A = U1[THM3Eqn::CONS_VAR_AREA];

  const Real e_inf = _fp.e_from_p_rho(_p_inf, _rho_inf);
  const Real E_inf = e_inf + 0.5 * _vel_inf * _vel_inf;

  flux.resize(THM3Eqn::N_EQ);
  flux[THM3Eqn::EQ_MASS] = _rho_inf * _vel_inf * A;
  flux[THM3Eqn::EQ_MOMENTUM] = (_rho_inf * _vel_inf * _vel_inf + _p_inf) * A;
  flux[THM3Eqn::EQ_ENERGY] = _vel_inf * (_rho_inf * E_inf + _p_inf) * A;
}

void
BoundaryFlux3EqnFreeInflow::calcJacobian(unsigned int /*iside*/,
                                         dof_id_type /*ielem*/,
                                         const std::vector<Real> & /*U1*/,
                                         const RealVectorValue & /*normal*/,
                                         DenseMatrix<Real> & jac1) const
{
  // Jacobian is zero
  jac1.resize(THM3Eqn::N_EQ, THM3Eqn::N_EQ);
}
