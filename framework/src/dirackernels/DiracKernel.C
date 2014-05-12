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

#include "DiracKernel.h"

// Moose includes
#include "SystemBase.h"
#include "Problem.h"

template<>
InputParameters validParams<DiracKernel>()
{
  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<NonlinearVariableName>("variable",
                                                 "The name of the variable that this kernel operates on");

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the displaced mesh for computation. "
                        "Note that in the case this is true but no displacements are provided in "
                        "the Mesh block the undisplaced mesh will still be used.");

  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.registerBase("DiracKernel");

  return params;
}

DiracKernel::DiracKernel(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    SetupInterface(parameters),
    CoupleableMooseVariableDependencyIntermediateInterface(parameters, false),
    FunctionInterface(parameters),
    UserObjectInterface(parameters),
    TransientInterface(parameters, name, "dirac_kernels"),
    MaterialPropertyInterface(name, parameters),
    PostprocessorInterface(parameters),
    GeometricSearchInterface(parameters),
    Restartable(name, parameters, "DiracKernels"),
    ZeroInterface(parameters),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _var(_sys.getVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _mesh(_subproblem.mesh()),
    _coord_sys(_assembly.coordSystem()),
    _dirac_kernel_info(_subproblem.diracKernelInfo()),
    _current_elem(_var.currentElem()),
    _q_point(_assembly.qPoints()),
    _physical_point(_assembly.physicalPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _phi(_assembly.phi()),
    _grad_phi(_assembly.gradPhi()),
    _test(_var.phi()),
    _grad_test(_var.gradPhi()),
    _u(_var.sln()),
    _grad_u(_var.gradSln()),
    _u_dot(_var.uDot()),
    _du_dot_du(_var.duDotDu())
{
  // Stateful material properties are not allowed on DiracKernels
  statefulPropertiesAllowed(false);
}

void
DiracKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    _current_point=_physical_point[_qp];
    if (isActiveAtPoint(_current_elem, _current_point))
    {
      for (_i = 0; _i < _test.size(); _i++)
        re(_i) += computeQpResidual();
    }
  }
}

void
DiracKernel::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    _current_point=_physical_point[_qp];
    if (isActiveAtPoint(_current_elem, _current_point))
      for (_i = 0; _i < _test.size(); _i++)
        for (_j = 0; _j < _phi.size(); _j++)
        {
          ke(_i, _j) += computeQpJacobian();
        }
  }
}

Real
DiracKernel::computeQpJacobian()
{
  return 0;
}

void
DiracKernel::addPoint(const Elem * elem, Point p, unsigned id)
{
  if (!elem || (elem->processor_id() != processor_id()))
    return;

  _dirac_kernel_info.addPoint(elem, p);
  _local_dirac_kernel_info.addPoint(elem, p);
}

const Elem *
DiracKernel::addPoint(Point p, unsigned id)
{
  if (id != libMesh::invalid_uint)
  {
    // OK, the user gave us an ID, let's see if we already have it...
    std::map<unsigned, std::pair<const Elem*, Point> >::iterator it = _point_cache.find(id);

    if (it != _point_cache.end())
    {
      // We have something cached, now make sure it's actually the same Point.
      // TODO: we should probably use this same comparison in the DiracKernelInfo code!
      Point cached_point = (it->second).second;

      if (cached_point.relative_fuzzy_equals(p))
      {
        // use cached data and call the other addPoint() method
        const Elem * cached_elem = (it->second).first;
        addPoint(cached_elem, p, id);
        return cached_elem;
      }
      else
        mooseError("Cached Dirac point " << cached_point << " already exists with ID: " << id << " and does not match point " << p);
    }
  }

  // If we made it here, we either didn't have the point already cached or
  // id == libMesh::invalid_uint.  So now do the expensive PointLocator lookup,
  // possibly cache the result, and call the other addPoint() method.
  AutoPtr<PointLocatorBase> pl = PointLocatorBase::build(TREE, _mesh);
  const Elem * elem = (*pl)(p);

  if (id != libMesh::invalid_uint)
  {
    // Add the point to the cache...
    _point_cache[id] = std::make_pair(elem, p);

    // ... and to the reverse cache.
    std::vector<std::pair<Point, unsigned> > & points = _reverse_point_cache[elem];
    points.push_back(std::make_pair(p, id));
  }

  addPoint(elem, p, id);
  return elem;
}

unsigned
DiracKernel::currentPointCachedID()
{
  std::map<const Elem*, std::vector<std::pair<Point, unsigned> > >::iterator it =
    _reverse_point_cache.find(_current_elem);

  // If the current Elem is not in the cache, return invalid_uint
  if (it == _reverse_point_cache.end())
    return libMesh::invalid_uint;

  // Do a linear search in the (hopefully small) vector of Points for this Elem
  std::vector<std::pair<Point, unsigned> > & points = it->second;

  std::vector<std::pair<Point, unsigned> >::iterator
    points_it = points.begin(),
    points_end = points.end();

  for (; points_it != points_end; ++points_it)
  {
    // If the current_point equals the cached point, return the associated id
    if (_current_point.relative_fuzzy_equals(points_it->first))
      return points_it->second;
  }

  // If we made it here, we didn't find the cached point, so return invalid_uint
  return libMesh::invalid_uint;
}

bool
DiracKernel::hasPointsOnElem(const Elem * elem)
{
  return _local_dirac_kernel_info.getElements().count(_mesh.elem(elem->id())) != 0;
}

bool
DiracKernel::isActiveAtPoint(const Elem * elem, const Point & p)
{
  return _local_dirac_kernel_info.hasPoint(elem, p);
}

void
DiracKernel::clearPoints()
{
  _local_dirac_kernel_info.clearPoints();
}

MooseVariable &
DiracKernel::variable()
{
  return _var;
}

SubProblem &
DiracKernel::subProblem()
{
  return _subproblem;
}
