/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACGrGrPolyStoredEnergy.h"
#include "GrainTrackerInterface.h"

template<>
InputParameters validParams<ACGrGrPolyStoredEnergy>()
{
  InputParameters params = ACBulk<Real>::validParams();
  params.addClassDescription("Stored energy contribution for the poly crystaline Allen-Cahn Kernel");
  params.addRequiredParam<std::vector<Real> >("stored_energy", "list of stored energies for each grain in the simulation");
  params.addRequiredParam<UserObjectName>("grain_tracker", "GrainTracker user object");
  params.addRequiredParam<unsigned int>("op_index", "Grain order parameter index for this kernel");
  return params;
}

ACGrGrPolyStoredEnergy::ACGrGrPolyStoredEnergy(const InputParameters & parameters) :
    ACBulk<Real>(parameters),
    _stored_energy(getParam<std::vector<Real> >("stored_energy")),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker")),
    _op_index(getParam<unsigned int>("op_index"))
{
}

Real
ACGrGrPolyStoredEnergy::computeDFDOP(PFFunctionType type)
{
  // both residual and Jacobian are zero outside of the [0:1] range
  if (_u[_qp] <= 0.0 || _u[_qp] >= 1.0)
    return 0.0;

  // ID of unique grain at current point and look up stored energy
  const std::vector<std::pair<unsigned int, unsigned int> > & grains = _grain_tracker.getElementalValues(_current_elem->id());
  for (unsigned int i = 0; i < grains.size(); ++i)
    if (grains[i].second == _op_index)
      switch (type)
      {
        case Residual:
          return 6.0 * _stored_energy[grains[i].first] * _u[_qp] * (1.0 - _u[_qp]);

        case Jacobian:
          return 6.0 * _stored_energy[grains[i].first] * (1.0 - 2.0 * _u[_qp]);

        default:
          mooseError("Invalid type passed in");
      }

  return 0.0;
}

Real
ACGrGrPolyStoredEnergy::computeQpOffDiagJacobian(unsigned int)
{
  return 0.0;
}
