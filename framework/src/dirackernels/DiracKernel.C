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

// Moose includes
#include "DiracKernel.h"
#include "Assembly.h"
#include "SystemBase.h"
#include "Problem.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<DiracKernel>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<MaterialPropertyInterface>();
  params.addRequiredParam<NonlinearVariableName>("variable",
                                                 "The name of the variable that this kernel operates on");

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the displaced mesh for computation. "
                        "Note that in the case this is true but no displacements are provided in "
                        "the Mesh block the undisplaced mesh will still be used.");

  params.addParam<bool>("drop_duplicate_points",
                        true,
                        "By default points added to a DiracKernel are dropped if a point at the same location"
                        "has been added before. If this option is set to false duplicate points are retained"
                        "and contribute to residual and Jacobian.");

  params.addParamNamesToGroup("use_displaced_mesh drop_duplicate_points", "Advanced");

  params.declareControllable("enable");
  params.registerBase("DiracKernel");

  params.set<Moose::MaterialDataType>("_material_data_type") = Moose::DIRAC_MATERIAL_DATA;

  return params;
}

DiracKernel::DiracKernel(const InputParameters & parameters) :
    MooseObject(parameters),
    SetupInterface(this),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    FunctionInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    MaterialPropertyInterface(this),
    PostprocessorInterface(this),
    GeometricSearchInterface(this),
    Restartable(parameters, "DiracKernels"),
    ZeroInterface(parameters),
    MeshChangedInterface(parameters),
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
    _du_dot_du(_var.duDotDu()),
    _drop_duplicate_points(parameters.get<bool>("drop_duplicate_points"))
{
}

void
DiracKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());

  const std::vector<unsigned int> * multiplicities = _drop_duplicate_points ? NULL : &_local_dirac_kernel_info.getPoints()[_current_elem].second;
  unsigned int local_qp = 0;
  Real multiplicity = 1.0;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    _current_point = _physical_point[_qp];
    if (isActiveAtPoint(_current_elem, _current_point))
    {
      if (!_drop_duplicate_points)
        multiplicity = (*multiplicities)[local_qp++];

      for (_i = 0; _i < _test.size(); _i++)
        re(_i) += multiplicity * computeQpResidual();
    }
  }
}

void
DiracKernel::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());

  const std::vector<unsigned int> * multiplicities = _drop_duplicate_points ? NULL : &_local_dirac_kernel_info.getPoints()[_current_elem].second;
  unsigned int local_qp = 0;
  Real multiplicity = 1.0;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    _current_point = _physical_point[_qp];
    if (isActiveAtPoint(_current_elem, _current_point))
    {
      if (!_drop_duplicate_points)
        multiplicity = (*multiplicities)[local_qp++];

      for (_i = 0; _i < _test.size(); _i++)
        for (_j = 0; _j < _phi.size(); _j++)
          ke(_i, _j) += multiplicity * computeQpJacobian();
    }
  }
}

void
DiracKernel::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number())
  {
    computeJacobian();
  }
  else
  {
    DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);

    const std::vector<unsigned int> * multiplicities = _drop_duplicate_points ? NULL : &_local_dirac_kernel_info.getPoints()[_current_elem].second;
    unsigned int local_qp = 0;
    Real multiplicity = 1.0;

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      _current_point = _physical_point[_qp];
      if (isActiveAtPoint(_current_elem, _current_point))
      {
        if (!_drop_duplicate_points)
          multiplicity = (*multiplicities)[local_qp++];

        for (_i=0; _i<_test.size(); _i++)
          for (_j=0; _j<_phi.size(); _j++)
            ke(_i, _j) += multiplicity * computeQpOffDiagJacobian(jvar);
      }
    }
  }
}

Real
DiracKernel::computeQpJacobian()
{
  return 0;
}

Real
DiracKernel::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0;
}

void
DiracKernel::addPoint(const Elem * elem, Point p, unsigned /*id*/)
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
    point_cache_t::iterator it = _point_cache.find(id);

    // Was the point found in a _point_cache on at least one processor?
    unsigned int i_found_it = static_cast<unsigned int>(it != _point_cache.end());
    unsigned int we_found_it = i_found_it;
    comm().max(we_found_it);

    // If the point was found in a cache, but not my cache, I'm not responsible for it.
    if (we_found_it && !i_found_it)
      return NULL;

    // Now that we only cache local data, some processors may enter
    // this if statement and some may not.  Therefore we can't call
    // any parallel_only() functions inside this if statement.
    if (i_found_it)
    {
      // We have something cached, now make sure it's actually the same Point.
      // TODO: we should probably use this same comparison in the DiracKernelInfo code!
      Point cached_point = (it->second).second;

      if (cached_point.relative_fuzzy_equals(p))
      {
        // Find the cached element associated to this point
        const Elem * cached_elem = (it->second).first;

        // If the cached element's processor ID doesn't match ours, we
        // are no longer responsible for caching it.  This can happen
        // due to adaptivity...
        if (cached_elem->processor_id() != processor_id())
        {
          // Update the caches, telling them to drop the cached Elem.
          // Analogously to the rest of the DiracKernel system, we
          // also return NULL because the Elem is non-local.
          updateCaches(cached_elem, NULL, p, id);
          return NULL;
        }

        bool active = cached_elem->active();
        bool contains_point = cached_elem->contains_point(p);

        // If the cached Elem is active and the point is still
        // contained in it, call the other addPoint() method and
        // return its result.
        if (active && contains_point)
        {
          // FIXME/TODO:
          // A given Point can be located in multiple elements if it
          // is on an edge or a node in the grid.  How are we handling
          // that case?  In other words, the same id would need to
          // appear multiple times in the _point_cache object...
          addPoint(cached_elem, p, id);
          return cached_elem;
        }

        // Is the Elem not active (been refined) but still contains the point?
        // Then search in its active children and update the caches.
        else if (!active && contains_point)
        {
          // Get the list of active children
          std::vector<const Elem*> active_children;
          cached_elem->active_family_tree(active_children);

          // Linear search through active children for the one that contains p
          for (unsigned c=0; c<active_children.size(); ++c)
            if (active_children[c]->contains_point(p))
            {
              updateCaches(cached_elem, active_children[c], p, id);
              addPoint(active_children[c], p, id);
              return active_children[c];
            }

          // If we got here without returning, it means the Point was
          // found in the parent element, but not in any of the active
          // children... this is not possible under normal
          // circumstances, so something must have gone seriously
          // wrong!
          mooseError("Error, Point not found in any of the active children!");
        }

        else if (
          // Is the Elem active but the point is not contained in it any
          // longer?  (For example, did the Mesh move out from under
          // it?)  Then we fall back to the expensive Point Locator
          // lookup.  TODO: we could try and do something more optimized
          // like checking if any of the active neighbors contains the
          // point.  Update the caches.
          (active && !contains_point) ||

          // The Elem has been refined *and* the Mesh has moved out
          // from under it, we fall back to doing the expensive Point
          // Locator lookup.  TODO: We could try and look in the
          // active children of this Elem's neighbors for the Point.
          // Update the caches.
          (!active && !contains_point))
        {
          const Elem * elem = _dirac_kernel_info.findPoint(p, _mesh);

          updateCaches(cached_elem, elem, p, id);
          addPoint(elem, p, id);
          return elem;
        }

        else
          mooseError("We'll never get here!");
      }
      else
        mooseError("Cached Dirac point " << cached_point << " already exists with ID: " << id << " and does not match point " << p);
    }
  }

  // If we made it here, we either didn't have the point already cached or
  // id == libMesh::invalid_uint.  So now do the more expensive PointLocator lookup,
  // possibly cache the result, and call the other addPoint() method.
  const Elem * elem = _dirac_kernel_info.findPoint(p, _mesh);

  // Only add the point to the cache on this processor if the Elem is local
  if (elem && (elem->processor_id() == processor_id()) && (id != libMesh::invalid_uint))
  {
    // Add the point to the cache...
    _point_cache[id] = std::make_pair(elem, p);

    // ... and to the reverse cache.
    std::vector<std::pair<Point, unsigned> > & points = _reverse_point_cache[elem];
    points.push_back(std::make_pair(p, id));
  }

  // Call the other addPoint() method.  This method ignores non-local
  // and NULL elements automatically.
  addPoint(elem, p, id);
  return elem;
}

unsigned
DiracKernel::currentPointCachedID()
{
  reverse_cache_t::iterator it = _reverse_point_cache.find(_current_elem);

  // If the current Elem is not in the cache, return invalid_uint
  if (it == _reverse_point_cache.end())
    return libMesh::invalid_uint;

  // Do a linear search in the (hopefully small) vector of Points for this Elem
  reverse_cache_t::mapped_type & points = it->second;

  for (const auto & points_it : points)
  {
    // If the current_point equals the cached point, return the associated id
    if (_current_point.relative_fuzzy_equals(points_it.first))
      return points_it.second;
  }

  // If we made it here, we didn't find the cached point, so return invalid_uint
  return libMesh::invalid_uint;
}

bool
DiracKernel::hasPointsOnElem(const Elem * elem)
{
  return _local_dirac_kernel_info.getElements().count(_mesh.elemPtr(elem->id())) != 0;
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

void
DiracKernel::updateCaches(const Elem* old_elem,
                          const Elem* new_elem,
                          Point p,
                          unsigned id)
{
  // Update the point cache.  Remove old cached data, only cache
  // new_elem if it is non-NULL and local.
  _point_cache.erase(id);
  if (new_elem && (new_elem->processor_id() == processor_id()))
    _point_cache[id] = std::make_pair(new_elem, p);

  // Update the reverse cache
  //
  // First, remove the Point from the old_elem's vector
  reverse_cache_t::iterator it = _reverse_point_cache.find(old_elem);
  if (it != _reverse_point_cache.end())
  {
    reverse_cache_t::mapped_type & points = it->second;
    {
      reverse_cache_t::mapped_type::iterator
        points_it = points.begin(),
        points_end = points.end();

      for (; points_it != points_end; ++points_it)
      {
        // If the point matches, remove it from the vector of points
        if (p.relative_fuzzy_equals(points_it->first))
        {
          // Vector erasure.  It can be slow but these vectors are
          // generally very short.  It also invalidates existing
          // iterators, so we need to break out of the loop and
          // not use them any more.
          points.erase(points_it);
          break;
        }
      }

      // If the points vector is now empty, remove old_elem from the reverse cache entirely
      if (points.empty())
        _reverse_point_cache.erase(old_elem);
    }
  }

  // Next, if new_elem is not NULL and local, add the point to the new_elem's vector
  if (new_elem && (new_elem->processor_id() == processor_id()))
  {
    reverse_cache_t::mapped_type & points = _reverse_point_cache[new_elem];
    points.push_back(std::make_pair(p, id));
  }
}
