//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "DiracKernelBase.h"
#include "Assembly.h"
#include "SystemBase.h"
#include "Problem.h"
#include "MooseMesh.h"

#include "libmesh/quadrature.h"

InputParameters
DiracKernelBase::validParams()
{
  InputParameters params = ResidualObject::validParams();
  params += MaterialPropertyInterface::validParams();
  params += BlockRestrictable::validParams();

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the displaced mesh for computation. "
                        "Note that in the case this is true but no displacements are provided in "
                        "the Mesh block the undisplaced mesh will still be used.");

  params.addParam<bool>(
      "drop_duplicate_points",
      true,
      "By default points added to a DiracKernel are dropped if a point at the same location"
      "has been added before. If this option is set to false duplicate points are retained"
      "and contribute to residual and Jacobian.");

  MooseEnum point_not_found_behavior("ERROR WARNING IGNORE", "IGNORE");
  params.addParam<MooseEnum>(
      "point_not_found_behavior",
      point_not_found_behavior,
      "By default (IGNORE), it is ignored if an added point cannot be located in the "
      "specified subdomains. If this option is set to ERROR, this situation will result in an "
      "error. If this option is set to WARNING, then a warning will be issued.");

  params.addParamNamesToGroup("use_displaced_mesh drop_duplicate_points", "Advanced");
  params.declareControllable("enable");
  return params;
}

DiracKernelBase::DiracKernelBase(const InputParameters & parameters)
  : ResidualObject(parameters),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    MaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, Moose::EMPTY_BOUNDARY_IDS),
    GeometricSearchInterface(this),
    BlockRestrictable(this),
    _current_elem(_assembly.elem()),
    _coord_sys(_assembly.coordSystem()),
    _dirac_kernel_info(_subproblem.diracKernelInfo()),
    _q_point(_assembly.qPoints()),
    _physical_point(_assembly.physicalPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _drop_duplicate_points(parameters.get<bool>("drop_duplicate_points")),
    _point_not_found_behavior(
        parameters.get<MooseEnum>("point_not_found_behavior").getEnum<PointNotFoundBehavior>())
{
  // Stateful material properties are not allowed on DiracKernels
  statefulPropertiesAllowed(false);
}

Real
DiracKernelBase::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0;
}

void
DiracKernelBase::addPoint(const Elem * elem, Point p, unsigned /*id*/)
{
  if (!elem || !hasBlocks(elem->subdomain_id()))
  {
    std::stringstream msg;
    msg << "Point " << p << " not found in block(s) " << Moose::stringify(blockIDs(), ", ") << ".";
    switch (_point_not_found_behavior)
    {
      case PointNotFoundBehavior::ERROR:
        mooseError(msg.str());
        break;
      case PointNotFoundBehavior::WARNING:
        mooseDoOnce(mooseWarning(msg.str()));
        break;
      case PointNotFoundBehavior::IGNORE:
        break;
      default:
        mooseError("Internal enum error.");
    }
    return;
  }

  if (elem->processor_id() != processor_id())
    return;

  _dirac_kernel_info.addPoint(elem, p);
  _local_dirac_kernel_info.addPoint(elem, p);
}

const Elem *
DiracKernelBase::addPoint(Point p, unsigned id)
{
  // Make sure that this method was called with the same id on all
  // processors.  It's an extra communication, though, so let's only
  // do it in DEBUG mode.
  libmesh_assert(comm().verify(id));

  if (id != libMesh::invalid_uint)
    return addPointWithValidId(p, id);

  // If id == libMesh::invalid_uint (the default), the user is not
  // enabling caching when they add Dirac points.  So all we can do is
  // the PointLocator lookup, and call the other addPoint() method.
  const Elem * elem = _dirac_kernel_info.findPoint(p, _mesh, blockIDs());
  addPoint(elem, p, id);
  return elem;
}

const Elem *
DiracKernelBase::addPointWithValidId(Point p, unsigned id)
{
  // The Elem we'll eventually return.  We can't return early on some
  // processors, because we may need to call parallel_only() functions in
  // the remainder of this scope.
  const Elem * return_elem = NULL;

  // May be set if the Elem is found in our cache, otherwise stays as NULL.
  const Elem * cached_elem = NULL;

  // OK, the user gave us an ID, let's see if we already have it...
  point_cache_t::iterator it = _point_cache.find(id);

  // Was the point found in a _point_cache on at least one processor?
  unsigned int i_found_it = static_cast<unsigned int>(it != _point_cache.end());
  unsigned int we_found_it = i_found_it;
  comm().max(we_found_it);

  // If nobody found it in their local caches, it means we need to
  // do the PointLocator look-up and update the caches.  This is
  // safe, because all processors have the same value of we_found_it.
  if (!we_found_it)
  {
    const Elem * elem = _dirac_kernel_info.findPoint(p, _mesh, blockIDs());

    // Only add the point to the cache on this processor if the Elem is local
    if (elem && (elem->processor_id() == processor_id()))
    {
      // Add the point to the cache...
      _point_cache[id] = std::make_pair(elem, p);

      // ... and to the reverse cache.
      std::vector<std::pair<Point, unsigned>> & points = _reverse_point_cache[elem];
      points.push_back(std::make_pair(p, id));
    }

    // Call the other addPoint() method.  This method ignores non-local
    // and NULL elements automatically.
    addPoint(elem, p, id);
    return_elem = elem;
  }

  // If the point was found in a cache, but not my cache, I'm not
  // responsible for it.
  //
  // We can't return early here: then we aren't allowed to call any more
  // parallel_only() functions in the remainder of this function!
  if (we_found_it && !i_found_it)
    return_elem = NULL;

  // This flag may be set by the processor that cached the Elem because it
  // needs to call findPoint() (due to moving mesh, etc.). If so, we will
  // call it at the end of the while loop below.
  bool i_need_find_point = false;

  // Now that we only cache local data, some processors may enter
  // this if statement and some may not.  Therefore we can't call
  // any parallel_only() functions inside this if statement.
  while (i_found_it)
  {
    // We have something cached, now make sure it's actually the same Point.
    // TODO: we should probably use this same comparison in the DiracKernelInfo code!
    Point cached_point = (it->second).second;

    if (cached_point.relative_fuzzy_equals(p))
    {
      // Find the cached element associated to this point
      cached_elem = (it->second).first;

      // If the cached element's processor ID doesn't match ours, we
      // are no longer responsible for caching it.  This can happen
      // due to adaptivity...
      if (cached_elem->processor_id() != processor_id())
      {
        // Update the caches, telling them to drop the cached Elem.
        // Analogously to the rest of the DiracKernel system, we
        // also return NULL because the Elem is non-local.
        updateCaches(cached_elem, NULL, p, id);
        return_elem = NULL;
        break; // out of while loop
      }

      bool active = cached_elem->active();
      bool contains_point = cached_elem->contains_point(p);

      // If the cached Elem is active and the point is still
      // contained in it, call the other addPoint() method and
      // return its result.
      if (active && contains_point)
      {
        addPoint(cached_elem, p, id);
        return_elem = cached_elem;
        break; // out of while loop
      }

      // Is the Elem not active (been refined) but still contains the point?
      // Then search in its active children and update the caches.
      else if (!active && contains_point)
      {
        // Get the list of active children
        std::vector<const Elem *> active_children;
        cached_elem->active_family_tree(active_children);

        // Linear search through active children for the one that contains p
        for (unsigned c = 0; c < active_children.size(); ++c)
          if (active_children[c]->contains_point(p))
          {
            updateCaches(cached_elem, active_children[c], p, id);
            addPoint(active_children[c], p, id);
            return_elem = active_children[c];
            break; // out of for loop
          }

        // If we got here without setting return_elem, it means the Point was
        // found in the parent element, but not in any of the active
        // children... this is not possible under normal
        // circumstances, so something must have gone seriously
        // wrong!
        if (!return_elem)
          mooseError("Error, Point not found in any of the active children!");

        break; // out of while loop
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
        i_need_find_point = true;
        break; // out of while loop
      }

      else
        mooseError("We'll never get here!");
    } // if (cached_point.relative_fuzzy_equals(p))
    else
      mooseError("Cached Dirac point ",
                 cached_point,
                 " already exists with ID: ",
                 id,
                 " and does not match point ",
                 p);

    // We only want one iteration of this while loop at maximum.
    i_found_it = false;
  } // while (i_found_it)

  // We are back to all processors here because we do not return
  // early in the code above...

  // Does we need to call findPoint() on all processors.
  unsigned int we_need_find_point = static_cast<unsigned int>(i_need_find_point);
  comm().max(we_need_find_point);

  if (we_need_find_point)
  {
    // findPoint() is a parallel-only function
    const Elem * elem = _dirac_kernel_info.findPoint(p, _mesh, blockIDs());

    updateCaches(cached_elem, elem, p, id);
    addPoint(elem, p, id);
    return_elem = elem;
  }

  return return_elem;
}

bool
DiracKernelBase::hasPointsOnElem(const Elem * elem)
{
  return _local_dirac_kernel_info.getElements().count(_mesh.elemPtr(elem->id())) != 0;
}

bool
DiracKernelBase::isActiveAtPoint(const Elem * elem, const Point & p)
{
  return _local_dirac_kernel_info.hasPoint(elem, p);
}

void
DiracKernelBase::clearPoints()
{
  _local_dirac_kernel_info.clearPoints();
}

void
DiracKernelBase::meshChanged()
{
  _point_cache.clear();
  _reverse_point_cache.clear();
}

void
DiracKernelBase::updateCaches(const Elem * old_elem, const Elem * new_elem, Point p, unsigned id)
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
      reverse_cache_t::mapped_type::iterator points_it = points.begin(), points_end = points.end();

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

unsigned
DiracKernelBase::currentPointCachedID()
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
