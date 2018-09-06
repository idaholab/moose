#include "BoundaryFlux3EqnGhostBase.h"

template <>
InputParameters
validParams<BoundaryFlux3EqnGhostBase>()
{
  InputParameters params = validParams<BoundaryFluxBase>();

  params.addClassDescription("Computes boundary fluxes for the 1-D, variable-area Euler equations "
                             "using a numerical flux user object and a ghost cell solution");

  params.addRequiredParam<UserObjectName>("numerical_flux", "Name of numerical flux user object");

  return params;
}

BoundaryFlux3EqnGhostBase::BoundaryFlux3EqnGhostBase(const InputParameters & parameters)
  : BoundaryFluxBase(parameters), _numerical_flux(getUserObject<RDGFluxBase>("numerical_flux"))
{
}

void
BoundaryFlux3EqnGhostBase::calcFlux(unsigned int iside,
                                    dof_id_type ielem,
                                    const std::vector<Real> & U1,
                                    const RealVectorValue & normal,
                                    std::vector<Real> & flux) const
{
  const std::vector<Real> U2 = getGhostCellSolution(U1);

  flux = _numerical_flux.getFlux(iside, ielem, U1, U2, normal, _tid);
}

void
BoundaryFlux3EqnGhostBase::calcJacobian(unsigned int iside,
                                        dof_id_type ielem,
                                        const std::vector<Real> & U1,
                                        const RealVectorValue & normal,
                                        DenseMatrix<Real> & J) const
{
  const std::vector<Real> U2 = getGhostCellSolution(U1);

  const auto & pdF_pdU1 = _numerical_flux.getJacobian(true, iside, ielem, U1, U2, normal, _tid);
  const auto & dF_dU2 = _numerical_flux.getJacobian(false, iside, ielem, U1, U2, normal, _tid);

  // compute dF/dU1 = pdF/pdU1 + dF/dU2 * dU2/dU1
  J = getGhostCellSolutionJacobian(U1);
  J.left_multiply(dF_dU2);
  J += pdF_pdU1;
}
