/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "FeatureVolumeVectorPostprocessor.h"
#include "FeatureFloodCount.h"
#include "GrainTrackerInterface.h"
#include "MooseMesh.h"
#include "Assembly.h"

#include "libmesh/quadrature.h"

template<>
InputParameters validParams<FeatureVolumeVectorPostprocessor>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();

  params.addRequiredParam<UserObjectName>("flood_counter", "The FeatureFloodCount UserObject to get values from.");
  return params;
}

FeatureVolumeVectorPostprocessor::FeatureVolumeVectorPostprocessor(const InputParameters & parameters) :
    GeneralVectorPostprocessor(parameters),
    MooseVariableDependencyInterface(),
    _flood_counter(getUserObject<FeatureFloodCount>("flood_counter")),
    _grain_volumes(declareVector("grain_volumes")),
    _vars(_flood_counter.getCoupledVars()),
    _mesh(_subproblem.mesh()),
    _assembly(_subproblem.assembly(_tid)),
    _q_point(_assembly.qPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation())
{
  addMooseVariableDependency(_vars);

  _coupled_sln.reserve(_vars.size());
  for (auto & var : _vars)
    _coupled_sln.push_back(&var->sln());
}

void
FeatureVolumeVectorPostprocessor::initialize()
{
}

void
FeatureVolumeVectorPostprocessor::execute()
{
  const GrainTrackerInterface * grain_tracker = dynamic_cast<const GrainTrackerInterface *>(&_flood_counter);

  if (!grain_tracker)
    mooseError("FeatureVolumeVectorPostprocessor currently only supports the GrainTrackerInterface");

  const auto num_grains = grain_tracker->getTotalNumberGrains();
  _grain_volumes.assign(num_grains, 0);

//  const FeatureFloodCount::FieldType field_type = FeatureFloodCount::FieldType::UNIQUE_REGION;

  const auto end = _mesh.getMesh().active_local_elements_end();
  for (auto el = _mesh.getMesh().active_local_elements_begin(); el != end; ++el)
  {
    const Elem * current_elem = *el;
    _fe_problem.prepare(current_elem, 0);
    _fe_problem.reinitElem(current_elem, 0);

    /**
     * Here we retrieve the op to grains or more generally the map of active
     * features at the current element location. We'll use that
     * information to figure out which variables are non-zero (from a
     * threshold perspective) then we can sum those values into
     * appropriate grain index locations.
     */
    const std::vector<unsigned int> & op_to_grain = grain_tracker->getOpToGrainsVector(current_elem->id());

    for (auto op_index = beginIndex(op_to_grain); op_index < op_to_grain.size(); ++op_index)
    {
      // Only sample "active" variables
      if (op_to_grain[op_index] != libMesh::invalid_uint)
      {
        auto grain_index = op_to_grain[op_index];
        mooseAssert(grain_index < num_grains, "Grain index out of range");

        // Add in the integral value on the current variable to the current feature's slot
        _grain_volumes[grain_index] += computeIntegral(op_index);
      }
    }
  }
}

void
FeatureVolumeVectorPostprocessor::finalize()
{
  // Do the parallel sum
  _communicator.sum(_grain_volumes);
}

Real
FeatureVolumeVectorPostprocessor::getGrainVolume(unsigned int grain_id) const
{
  mooseAssert(grain_id < _grain_volumes.size(), "grain_id is out of range");
  return _grain_volumes[grain_id];
}

Real
FeatureVolumeVectorPostprocessor::computeIntegral(std::size_t op_index) const
{
  Real sum = 0;

  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    sum += _JxW[qp] * _coord[qp] * (*_coupled_sln[op_index])[qp];

  return sum;
}
