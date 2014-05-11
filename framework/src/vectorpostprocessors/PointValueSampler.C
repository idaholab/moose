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

#include "PointValueSampler.h"

template<>
InputParameters validParams<PointValueSampler>()
{
  InputParameters params = validParams<SideVectorPostprocessor>();

  params += validParams<SamplerBase>();

  params.addRequiredCoupledVar("variable", "The names of the variables that this VectorPostprocessor operates on");

  params.addRequiredParam<std::vector<Point> >("points", "The points where you want to evaluate the variables");

  return params;
}

PointValueSampler::PointValueSampler(const std::string & name, InputParameters parameters) :
    GeneralVectorPostprocessor(name, parameters),
    CoupleableMooseVariableDependencyIntermediateInterface(parameters, false),
    SamplerBase(name, parameters, this, _communicator),
    _mesh(_subproblem.mesh()),
    _points(getParam<std::vector<Point> >("points")),
    _point_vec(1) // Only going to evaluate one point at a time for now
{
  std::vector<std::string> var_names(_coupled_moose_vars.size());
  _values.resize(_coupled_moose_vars.size());

  for (unsigned int i=0; i<_coupled_moose_vars.size(); i++)
    var_names[i] = _coupled_moose_vars[i]->name();

  // Initialize the datastructions in SamplerBase
  SamplerBase::setupVariables(var_names);
}

void
PointValueSampler::initialize()
{
  SamplerBase::initialize();

  // We do this here just in case it's been destroyed and recreated becaue of mesh adaptivity.
  _pl = _mesh.getMesh().sub_point_locator();
}

void
PointValueSampler::execute()
{
  for (unsigned int i=0; i<_points.size(); i++)
  {
    Point & p = _points[i];

    // First find the element the hit lands in
    const Elem * elem = getLocalElemContainingPoint(p, 0);

    // We have to pass a vector of points into reinitElemPhys
    _point_vec[0] = p;

    if (elem)
    {
      _subproblem.reinitElemPhys(elem, _point_vec, 0);

      for (unsigned int i=0; i<_coupled_moose_vars.size(); i++)
        _values[i] = _coupled_moose_vars[i]->sln()[0]; // The zero is for the "qp"

      SamplerBase::addSample(p, i, _values);
    }
  }
}

void
PointValueSampler::finalize()
{
  SamplerBase::finalize();
}

void
PointValueSampler::threadJoin(const UserObject & y)
{
  const PointValueSampler & vpp = static_cast<const PointValueSampler &>(y);

  SamplerBase::threadJoin(vpp);
}


const Elem *
PointValueSampler::getLocalElemContainingPoint(const Point & p, unsigned int id)
{
  const Elem * elem = (*_pl)(p);

  if (elem && elem->processor_id() == processor_id())
    return elem;

  return NULL;
}
