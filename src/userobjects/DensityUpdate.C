#include "DensityUpdate.h"
#include <algorithm>

registerMooseObject("troutApp", DensityUpdate);

InputParameters
DensityUpdate::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription("Compute updated densities based on sensitivities.");
  params.addRequiredParam<Real>("power", "Penalty power for SIMP method.");
  params.addRequiredCoupledVar("design_density", "Design density variable name.");
  params.addRequiredCoupledVar("filtered_design_density", "Filtered design density variable name.");
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
    _filtered_design_density(writableVariable("filtered_design_density")),
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
    _filtered_design_density.setNodalValue(elem_data.curr_filtered_density);
  }
  else
  {
    mooseError("Element data not found for the current element id.");
  }
}

void
DensityUpdate::finalize()
{
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
  Real l1 = 0;
  Real l2 = 1e9;
  bool perform_loop = true;
  while (perform_loop)
  {

    Real lmid = 0.5 * (l2 + l1);

    Real curr_total_volume = 0;
    for (auto && [id, elem_data] : _elem_data_map)
    {
      Real new_density = computeUpdatedDensity(elem_data.density, elem_data.sensitivity, lmid);
      elem_data.curr_filtered_density = new_density;
      curr_total_volume += new_density * elem_data.volume;
    }

    _communicator.sum(curr_total_volume);

    if (curr_total_volume > _total_allowable_volume)
      l1 = lmid;
    else
      l2 = lmid;

    // Prevent floating point difference.
    perform_loop = (l2 - l1) / (l1 + l2) > 1e-3;
    _communicator.broadcast(perform_loop);
  }
}

Real
DensityUpdate::computeUpdatedDensity(Real current_density, Real dc, Real lmid)
{
  Real move = 0.05;
  Real updated_density = std::max(
      0.0,
      std::min(1.0,
               std::min(current_density + move,
                        std::max(current_density - move,
                                 std::max(1e-5, current_density) * std::sqrt(-dc / lmid)))));
  return updated_density;
}
