/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PolyStoredEnergyAux.h"
#include "GrainTrackerInterface.h"

template<>
InputParameters validParams<PolyStoredEnergyAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Output stored energy contribution from ACGrGrPolyStoredEnergy for visialization");
  params.addRequiredParam<std::vector<Real> >("stored_energy", "list of stored energies for each grain in the simulation");
  params.addRequiredParam<UserObjectName>("grain_tracker", "GrainTracker user object");
  params.addRequiredCoupledVarWithAutoBuild("v", "var_name_base", "op_num", "Array of coupled variables");
  return params;
}

PolyStoredEnergyAux::PolyStoredEnergyAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _ncrys(coupledComponents("v")),
    _vals(_ncrys),
    _stored_energy(getParam<std::vector<Real> >("stored_energy")),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker"))
{
  // if (isNodal())
  //   mooseError("Use an elementa AuxVariable for PolyStoredEnergyAux.");

  for (unsigned int i = 0; i < _ncrys; ++i)
    _vals[i] = &coupledValue("v", i);
}

Real
PolyStoredEnergyAux::computeValue()
{
  Real energy = 0.0;

  // ID of unique grain at current point and look up stored energy
  const std::vector<std::pair<unsigned int, unsigned int> > & grains = _grain_tracker.getElementalValues(_current_elem->id());
  for (unsigned int i = 0; i < grains.size(); ++i)
  {
    mooseAssert(grains[i].first < _stored_energy.size(), "GrainTracker returned an invalid grain number.");
    mooseAssert(grains[i].second < _ncrys, "GrainTracker returned an invalid order parameter number.");

    // get phase field variable, clamped into [0:1]
    Real val = (*_vals[grains[i].second])[_qp];
    val = val < 0.0 ? 0.0 : (val > 1.0 ? 1.0 : val);

    // sum up stored energy contributions
    energy += _stored_energy[grains[i].first] * val * val * (3.0 - 2.0 * val);
  }

  return energy;
}
