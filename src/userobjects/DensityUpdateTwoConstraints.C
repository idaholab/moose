//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DensityUpdateTwoConstraints.h"
#include <algorithm>

registerMooseObject("troutApp", DensityUpdateTwoConstraints);

InputParameters
DensityUpdateTwoConstraints::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription(
      "Compute updated densities based on sensitivities using an optimality criteria method to "
      "keep the volume and cost constraints satisified.");
  params.addRequiredParam<Real>("power", "Penalty power for SIMP method.");
  params.addParam<Real>(
      "relative_tolerance",
      1.0e-3,
      "Relative tolerance on both compliance and cost to end the bisection method.");
  params.addRequiredCoupledVar("design_density", "Design density variable name.");
  params.addRequiredParam<VariableName>("density_sensitivity",
                                        "Name of the density_sensitivity variable.");
  params.addRequiredParam<VariableName>("cost_density_sensitivity",
                                        "Name of the cost density sensitivity variable.");
  params.addRequiredParam<Real>("volume_fraction", "Volume fraction");
  params.addRequiredParam<Real>("cost_fraction", "Cost fraction");
  params.addRequiredParam<VariableName>("cost", "Name of the cost variable.");

  params.addParam<Real>("bisection_lower_bound", 0, "Lower bound for the bisection algorithm.");
  params.addParam<Real>("bisection_upper_bound", 1e16, "Upper bound for the bisection algorithm.");
  return params;
}

DensityUpdateTwoConstraints::DensityUpdateTwoConstraints(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _mesh(_subproblem.mesh()),
    _density_sensitivity_name(getParam<VariableName>("density_sensitivity")),
    _cost_density_sensitivity_name(getParam<VariableName>("cost_density_sensitivity")),
    _cost_name(getParam<VariableName>("cost")),
    _design_density(writableVariable("design_density")),
    _density_sensitivity(_subproblem.getStandardVariable(_tid, _density_sensitivity_name)),
    _cost_density_sensitivity(
        _subproblem.getStandardVariable(_tid, _cost_density_sensitivity_name)),
    _cost(_subproblem.getStandardVariable(_tid, _cost_name)),
    _volume_fraction(getParam<Real>("volume_fraction")),
    _cost_fraction(getParam<Real>("cost_fraction")),
    _relative_tolerance(getParam<Real>("relative_tolerance")),
    _lower_bound(getParam<Real>("bisection_lower_bound")),
    _upper_bound(getParam<Real>("bisection_upper_bound"))
{
}

void
DensityUpdateTwoConstraints::timestepSetup()
{
  gatherElementData();
  performOptimCritLoop();
}

void
DensityUpdateTwoConstraints::execute()
{
  // Grab the element data for each id
  auto elem_data_iter = _elem_data_map.find(_current_elem->id());

  // Check if the element data is not null
  if (elem_data_iter != _elem_data_map.end())
  {
    ElementData & elem_data = elem_data_iter->second;
    _design_density.setNodalValue(elem_data.new_density);
  }
  else
  {
    mooseError("Element data not found for the current element id.");
  }
}

void
DensityUpdateTwoConstraints::gatherElementData()
{
  // Create parallel-consistent data structures constaining compliance and cost sensitivities.
  _elem_data_map.clear();
  _total_allowable_volume = 0;
  _total_allowable_cost = 0;

  for (const auto & elem : _mesh.getMesh().active_local_element_ptr_range())
  {
    dof_id_type elem_id = elem->id();
    ElementData data = ElementData(_design_density.getElementalValue(elem),
                                   _density_sensitivity.getElementalValue(elem),
                                   _cost_density_sensitivity.getElementalValue(elem),
                                   _cost.getElementalValue(elem),
                                   elem->volume(),
                                   0);
    _elem_data_map[elem_id] = data;
    _total_allowable_volume += elem->volume();
    _total_allowable_cost += 1.0;
  }

  _communicator.sum(_total_allowable_volume);
  _communicator.sum(_total_allowable_cost);

  _total_allowable_cost *= _cost_fraction;
  _total_allowable_volume *= _volume_fraction;
}

void
DensityUpdateTwoConstraints::performOptimCritLoop()
{
  // Initialize the lower and upper bounds for the bisection method
  Real l1 = _lower_bound;
  Real l2 = _upper_bound;

  // Initialize the lower and upper bounds for cost solution
  Real c1 = _lower_bound;
  Real c2 = _upper_bound;

  bool perform_loop = true;
  // Loop until the relative difference between l1 and l2 is less than a small tolerance
  Real loop_number = 0;

  while (perform_loop)
  {
    // Compute the midpoint between l1 and l2
    Real lmid = 0.5 * (l2 + l1);

    // Compute the midpoint between c1 and c2
    Real cmid = 0.5 * (c2 + c1);

    // Initialize the current total volume
    Real curr_total_volume = 0;

    // Initialize the current total cost
    Real curr_total_cost = 0;

    // Loop over all elements
    for (auto && [id, elem_data] : _elem_data_map)
    {
      // Compute the updated density for the current element
      Real new_density = computeUpdatedDensity(elem_data.old_density,
                                               elem_data.sensitivity,
                                               elem_data.cost_sensitivity,
                                               elem_data.cost,
                                               lmid,
                                               cmid,
                                               loop_number);

      // Update the current filtered density for the current element
      elem_data.new_density = new_density;
      // Update the current total volume
      curr_total_volume += new_density * elem_data.volume;
      // Update the current total cost
      curr_total_cost += new_density * elem_data.cost;
    }

    // Sum the current total volume across all processors
    _communicator.sum(curr_total_volume);
    _communicator.sum(curr_total_cost);

    // Update l1 or l2 based on whether the current total volume is greater than the total allowable
    // volume
    if (curr_total_volume > _total_allowable_volume)
      l1 = lmid;
    else
      l2 = lmid;

    if (curr_total_cost > _total_allowable_cost)
      c1 = cmid;
    else
      c2 = cmid;

    // Determine whether to continue the loop based on the relative difference between l1 and l2
    perform_loop =
        (l2 - l1) / (l1 + l2) > _relative_tolerance || (c2 - c1) / (c1 + c2) > _relative_tolerance;

    loop_number++;
  }
}

// Method to compute the updated density for an element
Real
DensityUpdateTwoConstraints::computeUpdatedDensity(Real current_density,
                                                   Real dc,
                                                   Real cost_sensitivity,
                                                   Real cost,
                                                   Real lmid,
                                                   Real cmid,
                                                   Real loop_number)
{
  // Define the maximum allowable change in density
  const Real move_min = 1.0e-3;
  const Real move_zero = 0.15;
  const Real alpha = 0.16;

  Real move = 0.1;
  // move = std::max(MathUtils::pow(alpha, loop_number) * move_zero, move_min);

  Real denominator = lmid + cmid * cost + cmid * current_density * cost_sensitivity;

  // Compute the updated density based on the current density, the sensitivity, and the midpoint
  // value
  // Real updated_density = std::max(
  //     0.0,
  //     std::min(1.0,
  //              std::min(current_density + move,
  //                       std::max(current_density - move,
  //                                std::max(1e-5, current_density) * std::sqrt(-dc /
  //                                denominator)))));

  Real updated_density = std::max(
      1.0e-5,
      std::max(current_density - move,
               std::min(1.0,
                        std::min(current_density + move,
                                 current_density * std::sqrt(std::abs(-dc / denominator))))));

  // Return the updated density
  return updated_density;
}
