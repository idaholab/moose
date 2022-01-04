#include "ADBoundaryFlux3EqnFreeOutflow.h"
#include "THMIndices3Eqn.h"
#include "Numerics.h"

registerMooseObject("THMApp", ADBoundaryFlux3EqnFreeOutflow);

InputParameters
ADBoundaryFlux3EqnFreeOutflow::validParams()
{
  InputParameters params = ADBoundaryFluxBase::validParams();

  params.addClassDescription("Computes the outflow boundary flux directly for the 1-D, 1-phase, "
                             "variable-area Euler equations");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of fluid properties user object");

  return params;
}

ADBoundaryFlux3EqnFreeOutflow::ADBoundaryFlux3EqnFreeOutflow(const InputParameters & parameters)
  : ADBoundaryFluxBase(parameters),

    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

void
ADBoundaryFlux3EqnFreeOutflow::calcFlux(unsigned int /*iside*/,
                                        dof_id_type /*ielem*/,
                                        const std::vector<ADReal> & U1,
                                        const RealVectorValue & /*normal*/,
                                        std::vector<ADReal> & flux) const
{
  // extract the solution and area
  const ADReal rhoA1 = U1[THM3Eqn::CONS_VAR_RHOA];
  const ADReal rhouA1 = U1[THM3Eqn::CONS_VAR_RHOUA];
  const ADReal rhoEA1 = U1[THM3Eqn::CONS_VAR_RHOEA];
  const ADReal A1 = U1[THM3Eqn::CONS_VAR_AREA];

  const ADReal rho1 = rhoA1 / A1;
  const ADReal vel1 = rhouA1 / rhoA1;
  const ADReal v1 = 1.0 / rho1;
  const ADReal e1 = rhoEA1 / rhoA1 - 0.5 * vel1 * vel1;
  const ADReal p1 = _fp.p_from_v_e(v1, e1);

  flux.resize(THM3Eqn::N_EQ);
  flux[THM3Eqn::EQ_MASS] = vel1 * rhoA1;
  flux[THM3Eqn::EQ_MOMENTUM] = (vel1 * rhouA1 + p1 * A1);
  flux[THM3Eqn::EQ_ENERGY] = vel1 * (rhoEA1 + p1 * A1);
}
