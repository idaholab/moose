//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DensityUpdate.h"
#include <algorithm>

registerMooseObject("troutApp", DensityUpdate);

InputParameters
DensityUpdate::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription(
      "Compute updated densities based on sensitivities using an optimality criteria method to "
      "keep the volume constraint satisified.");
  params.addRequiredParam<Real>("power", "Penalty power for SIMP method.");
  params.addRequiredCoupledVar("design_density", "Design density variable name.");
  params.addRequiredParam<VariableName>("density_sensitivity",
                                        "Name of the density_sensitivity variable.");
  params.addRequiredParam<Real>("volume_fraction", "Volume Fraction");
  return params;
}

DensityUpdate::DensityUpdate(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _mesh(_subproblem.mesh()),
    _density_sensitivity_name(getParam<VariableName>("density_sensitivity")),
    _design_density(writableVariable("design_density")),
    _density_sensitivity(_subproblem.getStandardVariable(_tid, _density_sensitivity_name)),
    _volume_fraction(getParam<Real>("volume_fraction"))
{
}

void
DensityUpdate::timestepSetup()
{
  gatherElementData();
  performOptimCritLoop();
}

void
DensityUpdate::execute()
{
  // Grab the element data for each id
  auto elem_data_iter = _elem_data_map.find(_current_elem->id());

  // Check if the element data is not null
  if (elem_data_iter != _elem_data_map.end())
  {
    ElementData & elem_data = elem_data_iter->second;
    _design_density.setNodalValue(elem_data.curr_filtered_density);
  }
  else
  {
    mooseError("Element data not found for the current element id.");
  }
}

void
DensityUpdate::gatherElementData()
{
  _elem_data_map.clear();
  _total_allowable_volume = 0;
  for (const auto & elem : _mesh.getMesh().active_local_element_ptr_range())
  {
    dof_id_type elem_id = elem->id();
    ElementData data = ElementData(_design_density.getElementalValue(elem),
                                   _density_sensitivity.getElementalValue(elem),
                                   elem->volume(),
                                   0);
    _elem_data_map[elem_id] = data;
    _total_allowable_volume += elem->volume();
  }

  _communicator.sum(_total_allowable_volume);
  _total_allowable_volume *= _volume_fraction;
}

void
DensityUpdate::performOptimCritLoop()
{
  // Initialize the lower and upper bounds for the bisection method
  Real l1 = 0;
  Real l2 = 1e16;
  bool perform_loop = true;
  // Loop until the relative difference between l1 and l2 is less than a small tolerance
  while (perform_loop)
  {
    // Compute the midpoint between l1 and l2
    Real lmid = 0.5 * (l2 + l1);

    // Initialize the current total volume
    Real curr_total_volume = 0;
    // Loop over all elements
    for (auto && [id, elem_data] : _elem_data_map)
    {
      // Compute the updated density for the current element
      Real new_density = computeUpdatedDensity(elem_data.density, elem_data.sensitivity, lmid);
      // Update the current filtered density for the current element
      elem_data.curr_filtered_density = new_density;
      // Update the current total volume
      curr_total_volume += new_density * elem_data.volume;
    }

    // Sum the current total volume across all processors
    _communicator.sum(curr_total_volume);

    // Update l1 or l2 based on whether the current total volume is greater than the total allowable
    // volume
    if (curr_total_volume > _total_allowable_volume)
      l1 = lmid;
    else
      l2 = lmid;

    // Determine whether to continue the loop based on the relative difference between l1 and l2
    perform_loop = (l2 - l1) / (l1 + l2) > 1e-3;
    // Broadcast the decision to continue the loop to all processors
    _communicator.broadcast(perform_loop);
  }
}

// Method to compute the updated density for an element
Real
DensityUpdate::computeUpdatedDensity(Real current_density, Real dc, Real lmid)
{
  // Define the maximum allowable change in density
  Real move = 0.5;
  // Compute the updated density based on the current density, the sensitivity, and the midpoint
  // value
  Real updated_density = std::max(
      0.0,
      std::min(1.0,
               std::min(current_density + move,
                        std::max(current_density - move,
                                 std::max(1e-5, current_density) * std::sqrt(-dc / lmid)))));
  // Return the updated density
  return updated_density;
}
