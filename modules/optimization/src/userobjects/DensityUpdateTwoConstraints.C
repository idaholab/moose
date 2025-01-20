//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DensityUpdateTwoConstraints.h"
#include "Transient.h"
#include <algorithm>

registerMooseObject("OptimizationApp", DensityUpdateTwoConstraints);

InputParameters
DensityUpdateTwoConstraints::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription(
      "Compute updated densities based on sensitivities using an optimality criteria method to "
      "keep the volume and cost constraints satisified.");
  params.addParam<Real>(
      "relative_tolerance",
      1.0e-3,
      "Relative tolerance on both compliance and cost to end the bisection method.");
  params.addRequiredCoupledVar("design_density", "Design density variable name.");
  params.addRequiredParam<VariableName>("density_sensitivity",
                                        "Name of the density_sensitivity variable.");
  params.addRequiredParam<VariableName>("cost_density_sensitivity",
                                        "Name of the cost density sensitivity variable.");
  params.addParam<VariableName>("thermal_sensitivity",
                                "Name of the thermal density sensitivity variable.");
  params.addRequiredParam<Real>("volume_fraction", "Volume fraction.");
  params.addRequiredParam<Real>("cost_fraction", "Cost fraction.");
  params.addParam<Real>("bisection_move", 0.01, "Bisection move for the updated solution.");
  params.addParam<bool>("adaptive_move",
                        false,
                        "Whether incremental moves in the bisection algorithm will be reduced as "
                        "the number of iterations increases. Note that the time must correspond to "
                        "iteration number for better results.");
  params.addRequiredParam<VariableName>("cost", "Name of the cost variable.");

  params.addParam<Real>("bisection_lower_bound", 0, "Lower bound for the bisection algorithm.");
  params.addParam<Real>("bisection_upper_bound", 1e16, "Upper bound for the bisection algorithm.");

  params.addParam<std::vector<Real>>(
      "weight_mechanical_thermal",
      "List of values between 0 and 1 to weight the stiffness and thermal sensitivities");

  params.addParam<bool>("use_thermal_compliance",
                        false,
                        "Whether to include the thermal compliance in the sensitivities to "
                        "minimize in conjunction with stiffness compliance.");

  return params;
}

DensityUpdateTwoConstraints::DensityUpdateTwoConstraints(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _mesh(_subproblem.mesh()),
    _density_sensitivity_name(getParam<VariableName>("density_sensitivity")),
    _cost_density_sensitivity_name(getParam<VariableName>("cost_density_sensitivity")),
    _cost_name(getParam<VariableName>("cost")),
    _design_density(&writableVariable("design_density")),
    _density_sensitivity(&_subproblem.getStandardVariable(_tid, _density_sensitivity_name)),
    _cost_density_sensitivity(
        &_subproblem.getStandardVariable(_tid, _cost_density_sensitivity_name)),
    _cost(_subproblem.getStandardVariable(_tid, _cost_name)),
    _volume_fraction(getParam<Real>("volume_fraction")),
    _cost_fraction(getParam<Real>("cost_fraction")),
    _relative_tolerance(getParam<Real>("relative_tolerance")),
    _lower_bound(getParam<Real>("bisection_lower_bound")),
    _upper_bound(getParam<Real>("bisection_upper_bound")),
    _bisection_move(getParam<Real>("bisection_move")),
    _adaptive_move(getParam<bool>("adaptive_move")),
    _thermal_sensitivity_name(
        isParamValid("thermal_sensitivity") ? getParam<VariableName>("thermal_sensitivity") : ""),
    _thermal_sensitivity(isParamValid("thermal_sensitivity")
                             ? &_subproblem.getStandardVariable(_tid, _thermal_sensitivity_name)
                             : nullptr)
{
  if (isParamValid("thermal_sensitivity"))
  {
    if (!isParamValid("thermal_sensitivity"))
      paramError("thermal_sensitivity",
                 "The thermal_sensitivity variable name must be provided by the user if "
                 "include_thermal_compliance is chosen to be true.");

    if (isParamValid("weight_mechanical_thermal"))
    {
      _weight_values = getParam<std::vector<Real>>("weight_mechanical_thermal");
      if (_weight_values.size() != 2)
        paramError("weight_mechanical_thermal",
                   "Weighing of sensitivities is only available for the mechanical compliance and "
                   "the thermal compliances problems, respectively.");
    }
    else
      paramError("weight_mechanical_thermal",
                 "This parameter needs to be provided when including thermal sensitivity.");
  }

  auto transient = dynamic_cast<TransientBase *>(_app.getExecutioner());

  if (!transient && _adaptive_move)
    paramError("adaptive_move", "Cannot find a transient executioner for adaptive bisection move.");

  if (!dynamic_cast<MooseVariableFE<Real> *>(_design_density))
    paramError("design_density", "Design density must be a finite element variable");

  if (!dynamic_cast<const MooseVariableFE<Real> *>(_density_sensitivity))
    paramError("density_sensitivity", "Design sensitivity must be a finite element variable");

  if (!dynamic_cast<const MooseVariableFE<Real> *>(_cost_density_sensitivity))
    paramError("cost_density_sensitivity",
               "Cost density sensitivity must be a finite element variable");
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
    _design_density->setNodalValue(elem_data.new_density);
  }
  else
  {
    mooseError("Element data not found for the current element id.");
  }
}

void
DensityUpdateTwoConstraints::gatherElementData()
{
  // Create parallel-consistent data structures constraining compliance and cost sensitivities.
  _elem_data_map.clear();
  _total_allowable_volume = 0;
  _total_allowable_cost = 0;

  for (const auto & sub_id : blockIDs())
    for (const auto & elem : _mesh.getMesh().active_local_subdomain_elements_ptr_range(sub_id))
    {
      dof_id_type elem_id = elem->id();
      ElementData data = ElementData(
          dynamic_cast<MooseVariableFE<Real> *>(_design_density)->getElementalValue(elem),
          dynamic_cast<const MooseVariableFE<Real> *>(_density_sensitivity)
              ->getElementalValue(elem),
          dynamic_cast<const MooseVariableFE<Real> *>(_cost_density_sensitivity)
              ->getElementalValue(elem),
          isParamValid("thermal_sensitivity") ? _thermal_sensitivity->getElementalValue(elem) : 0.0,
          _cost.getElementalValue(elem),
          elem->volume(),
          0);
      _elem_data_map[elem_id] = data;
      _total_allowable_volume += elem->volume();
    }

  _communicator.sum(_total_allowable_volume);
  _communicator.sum(_total_allowable_cost);

  _total_allowable_cost = _cost_fraction * _total_allowable_volume;
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
      Real new_density = computeUpdatedDensity(
          elem_data.old_density,
          elem_data.sensitivity * (isParamValid("thermal_sensitivity") ? _weight_values[0] : 1.0),
          elem_data.cost_sensitivity,
          elem_data.thermal_sensitivity *
              (isParamValid("thermal_sensitivity") ? _weight_values[1] : 1.0),
          elem_data.cost,
          lmid,
          cmid);

      // Update the current filtered density for the current element
      elem_data.new_density = new_density;
      // Update the current total volume
      curr_total_volume += new_density * elem_data.volume;
      // Update the current total cost
      curr_total_cost += new_density * elem_data.volume * elem_data.cost;
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
  }
}

// Method to compute the updated density for an element
Real
DensityUpdateTwoConstraints::computeUpdatedDensity(Real current_density,
                                                   Real dc,
                                                   Real cost_sensitivity,
                                                   Real temp_sensitivity,
                                                   Real cost,
                                                   Real lmid,
                                                   Real cmid)
{
  Real move = _bisection_move;

  // Minimum move
  const Real move_min = 1.0e-3;
  // Control adaptivity
  const Real alpha = 0.96;

  // Adaptive move (takes the transient time step as SIMP iteration number)
  if (_adaptive_move)
    move = std::max(MathUtils::pow(alpha, _t) * move, move_min);

  Real denominator = lmid + cmid * cost + cmid * current_density * cost_sensitivity;

  // Effect of damping to be assessed
  const Real damping = 1.0;

  Real updated_density = std::max(
      0.0,
      std::max(
          current_density - move,
          std::min(1.0,
                   std::min(current_density + move,
                            current_density *
                                MathUtils::pow(std::sqrt((-(dc + temp_sensitivity) / denominator)),
                                               damping)))));

  // Return the updated density
  return updated_density;
}
