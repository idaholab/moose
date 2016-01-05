/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACGrGrPolyStoredEnergy.h"

template<>
InputParameters validParams<ACGrGrPolyStoredEnergy>()
{
  InputParameters params = validParams<ACBulk>();
  params.addClassDescription("Stored energy contribution for the poly crystaline Allen-Cahn Kernel");
  params.addRequiredParam<std::vector<Real> >("stored_energy", "list of stored energies for each grain in the simulation");
  params.addRequiredParam<UserObjectName>("grain_tracker", "GrainTracker user object");
  params.addRequiredParam<unsigned int>("op_index", "Grain order parameter index for this kernel");
  return params;
}

ACGrGrPolyStoredEnergy::ACGrGrPolyStoredEnergy(const InputParameters & parameters) :
    ACBulk(parameters),
    _stored_energy(getParam<std::vector<Real> >("stored_energy")),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker")),
    _op_index(getParam<unsigned int>("op_index"))
{
}

Real
ACGrGrPolyStoredEnergy::computeDFDOP(PFFunctionType type)
{
  // Calculate either the residual or Jacobian of the stored energy
  switch (type)
  {
    case Residual:
    {
      // ID of unique grain at current point and look up stored energy
      const std::vector<std::pair<unsigned int, unsigned int> > & grains = _grain_tracker.getElementalValues(_current_elem->id());
      for (unsigned int i = 0; i < grains.size(); ++i)
        if (grains[i].second == _op_index)
          return _stored_energy[grains[i].first];

      return 0.0;
    }

    case Jacobian:
      return 0.0;

    default:
      mooseError("Invalid type passed in");
  }
}

Real
ACGrGrPolyStoredEnergy::computeQpOffDiagJacobian(unsigned int)
{
  return 0.0;
}
