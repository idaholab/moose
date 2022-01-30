#include "ADBoundaryFlux3EqnGhostBase.h"
#include "ADNumericalFlux3EqnBase.h"

InputParameters
ADBoundaryFlux3EqnGhostBase::validParams()
{
  InputParameters params = ADBoundaryFluxBase::validParams();

  params.addClassDescription("Computes boundary fluxes for the 1-D, variable-area Euler equations "
                             "using a numerical flux user object and a ghost cell solution");

  params.addRequiredParam<UserObjectName>("numerical_flux", "Name of numerical flux user object");
  params.addRequiredParam<Real>("normal", "Outward normal");

  return params;
}

ADBoundaryFlux3EqnGhostBase::ADBoundaryFlux3EqnGhostBase(const InputParameters & parameters)
  : ADBoundaryFluxBase(parameters),
    _numerical_flux(getUserObject<ADNumericalFlux3EqnBase>("numerical_flux")),
    _normal(getParam<Real>("normal"))
{
}

void
ADBoundaryFlux3EqnGhostBase::calcFlux(unsigned int iside,
                                      dof_id_type ielem,
                                      const std::vector<ADReal> & U1,
                                      const RealVectorValue & normal,
                                      std::vector<ADReal> & flux) const
{
  const std::vector<ADReal> U2 = getGhostCellSolution(U1);

  flux = _numerical_flux.getFlux(iside, ielem, true, U1, U2, normal(0));
}
