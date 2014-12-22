/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "PointSamplerBase.h"
#include "MooseUtils.h"

template<>
InputParameters validParams<PointSamplerBase>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();

  params += validParams<SamplerBase>();

  params.addRequiredCoupledVar("variable", "The names of the variables that this VectorPostprocessor operates on");

  return params;
}

PointSamplerBase::PointSamplerBase(const std::string & name, InputParameters parameters) :
    GeneralVectorPostprocessor(name, parameters),
    CoupleableMooseVariableDependencyIntermediateInterface(parameters, false),
    SamplerBase(name, parameters, this, _communicator),
    _mesh(_subproblem.mesh().getMesh()),
    _local_ids(n_processors())
{
  std::vector<std::string> var_names(_coupled_moose_vars.size());
  _values.resize(_coupled_moose_vars.size());

  for (unsigned int i=0; i<_coupled_moose_vars.size(); i++)
    var_names[i] = _coupled_moose_vars[i]->name();

  // Initialize the datastructions in SamplerBase
  SamplerBase::setupVariables(var_names);
}

void
PointSamplerBase::initialize()
{
  SamplerBase::initialize();

  // We do this here just in case it's been destroyed and recreated because of mesh adaptivity.
  _point_locator = _mesh.sub_point_locator();

  // The following code populates the local point indices for more efficient parallel calls to the point locator in execute() method
  // Computes the number local parts
  double n = n_processors(); // num. of procs, use Real to get correct division below
  unsigned int parts = std::ceil(_ids.size() / n);

  // Loop through the ids and store a subset of the ids for each processor
  unsigned int cnt = 0;
  for (std::vector<Real>::iterator it = _ids.begin(); it < _ids.end(); it+=parts)
  {
    if (it+parts < _ids.end())
      _local_ids[cnt] = std::vector<Real>(it, it+parts);
    else
      _local_ids[cnt] = std::vector<Real>(it, _ids.end());
    cnt += 1;
  }

}

void
PointSamplerBase::execute()
{

  // Clear any previously located element and root ids
  _elem_ids.clear();
  _root_ids.clear();

  // Loop over the points assigned to this processor
  for (std::vector<Real>::const_iterator it = _local_ids[processor_id()].begin(); it != _local_ids[processor_id()].end(); ++it)
  {
    Point & p = _points[*it];

    // First find the element the hit lands in
    const Elem * elem = (*_point_locator)(p);

    // Error if the element cannot be located
    if (!elem)
      mooseError("No element located at " << p << " in PointSamplerBase VectorPostprocessor named: " << _name);

    // Store the elem id and its processor
    _elem_ids.push_back(elem->id());
    _root_ids.push_back(elem->processor_id());
  }

}

void
PointSamplerBase::finalize()
{
  // Build a single vector of all the located elements
  _communicator.allgather(_elem_ids);
  _communicator.allgather(_root_ids);

  // A vector version of the current point, this is needed for reinintElemPhys
  std::vector<Point> point_vec(1);

  // Loop over all located elements, compute the variable values, and add the values to the sampling vectors (see SamplerBase)
  for (unsigned int i = 0; i < _root_ids.size(); ++i)
    if (_root_ids[i] == processor_id())
    {
      const Elem * elem = _mesh.elem(_elem_ids[i]);

      point_vec[0] = _points[i];
      _subproblem.reinitElemPhys(elem, point_vec, 0); // Zero is for tid

      for (unsigned int j=0; j<_coupled_moose_vars.size(); j++)
        _values[j] = _coupled_moose_vars[j]->sln()[0]; // The zero is for the "qp"

      SamplerBase::addSample(_points[i], _ids[i], _values);
    }

  SamplerBase::finalize();
}

void
PointSamplerBase::threadJoin(const SamplerBase & y)
{
  const PointSamplerBase & vpp = static_cast<const PointSamplerBase &>(y);
  SamplerBase::threadJoin(vpp);
}
