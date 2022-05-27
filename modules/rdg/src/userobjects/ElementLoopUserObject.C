//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementLoopUserObject.h"

InputParameters
ElementLoopUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += BlockRestrictable::validParams();
  return params;
}

ElementLoopUserObject::ElementLoopUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    BlockRestrictable(this),
    Coupleable(this, false),
    MooseVariableDependencyInterface(this),
    _mesh(_subproblem.mesh()),
    _current_elem(_assembly.elem()),
    _current_elem_volume(_assembly.elemVolume()),
    _q_point(_assembly.qPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation()),
    _have_interface_elems(false)
{
  // Keep track of which variables are coupled so we know what we depend on
  const std::vector<MooseVariableFEBase *> & coupled_vars = getCoupledMooseVars();
  for (unsigned int i = 0; i < coupled_vars.size(); i++)
    addMooseVariableDependency(coupled_vars[i]);
}

ElementLoopUserObject::ElementLoopUserObject(ElementLoopUserObject & x, Threads::split /*split*/)
  : GeneralUserObject(x.parameters()),
    BlockRestrictable(&x),
    Coupleable(this, false),
    MooseVariableDependencyInterface(this),
    _mesh(x._subproblem.mesh()),
    _current_elem(x._assembly.elem()),
    _current_elem_volume(x._assembly.elemVolume()),
    _q_point(x._assembly.qPoints()),
    _qrule(x._assembly.qRule()),
    _JxW(x._assembly.JxW()),
    _coord(x._assembly.coordTransformation()),
    _have_interface_elems(false)
{
  // Keep track of which variables are coupled so we know what we depend on
  const std::vector<MooseVariableFEBase *> & coupled_vars = x.getCoupledMooseVars();
  for (unsigned int i = 0; i < coupled_vars.size(); i++)
    addMooseVariableDependency(coupled_vars[i]);
}

ElementLoopUserObject::~ElementLoopUserObject() {}

void
ElementLoopUserObject::initialize()
{
}

void
ElementLoopUserObject::preElement(const Elem * elem)
{
  _fe_problem.setCurrentSubdomainID(elem, _tid);
}

void
ElementLoopUserObject::execute()
{
  ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();

  try
  {
    pre();

    _subdomain = std::numeric_limits<SubdomainID>::max();
    ConstElemRange::const_iterator el = elem_range.begin();
    for (el = elem_range.begin(); el != elem_range.end(); ++el)
    {
      if (!keepGoing())
        break;

      const Elem * elem = *el;
      unsigned int cur_subdomain = elem->subdomain_id();
      preElement(elem);

      _old_subdomain = _subdomain;
      _subdomain = cur_subdomain;

      if (this->hasBlocks(_subdomain))
      {
        if (_subdomain != _old_subdomain)
          subdomainChanged();

        onElement(elem);

        for (unsigned int side = 0; side < elem->n_sides(); side++)
        {
          std::vector<BoundaryID> boundary_ids = _mesh.getBoundaryIDs(elem, side);

          if (boundary_ids.size() > 0)
            for (std::vector<BoundaryID>::iterator it = boundary_ids.begin();
                 it != boundary_ids.end();
                 ++it)
              onBoundary(elem, side, *it);

          if (elem->neighbor_ptr(side) != NULL)
          {
            if (this->hasBlocks(elem->neighbor_ptr(side)->subdomain_id()))
              onInternalSide(elem, side);
            if (boundary_ids.size() > 0)
              for (std::vector<BoundaryID>::iterator it = boundary_ids.begin();
                   it != boundary_ids.end();
                   ++it)
                onInterface(elem, side, *it);
          }
        } // sides
      }
    } // range

    post();
  }
  catch (MooseException & e)
  {
    caughtMooseException(e);
  }
}

void
ElementLoopUserObject::finalize()
{
  _have_interface_elems = true;
}

void
ElementLoopUserObject::pre()
{
}

void
ElementLoopUserObject::subdomainChanged()
{
}

void
ElementLoopUserObject::onElement(const Elem * elem)
{
  _current_elem = elem;
  computeElement();
}

void
ElementLoopUserObject::onBoundary(const Elem * /*elem*/, unsigned int side, BoundaryID /*bnd_id*/)
{
  _current_side = side;
  computeBoundary();
}

void
ElementLoopUserObject::onInternalSide(const Elem * elem, unsigned int side)
{
  _current_elem = elem;
  // Pointer to the neighbor we are currently working on.
  _current_neighbor = elem->neighbor_ptr(side);

  // Get the global id of the element and the neighbor
  const dof_id_type elem_id = elem->id();
  const dof_id_type neighbor_id = _current_neighbor->id();

  // TODO: add if-statement to check if this needs to be executed
  if ((_current_neighbor->active() && (_current_neighbor->level() == elem->level()) &&
       (elem_id < neighbor_id)) ||
      (_current_neighbor->level() < elem->level()))
  {
    computeInternalSide();
  }

  if (!_have_interface_elems &&
      (_current_elem->processor_id() != _current_neighbor->processor_id()))
  {
    // if my current neighbor is on another processor store the current element ID for later
    // communication
    _interface_elem_ids.insert(_current_elem->id());
  }
}

void
ElementLoopUserObject::onInterface(const Elem * elem, unsigned int side, BoundaryID /*bnd_id*/)
{
  _current_elem = elem;
  // Pointer to the neighbor we are currently working on.
  _current_neighbor = elem->neighbor_ptr(side);

  // Get the global id of the element and the neighbor
  const dof_id_type elem_id = elem->id();
  const dof_id_type neighbor_id = _current_neighbor->id();

  // TODO: add if-statement to check if this needs to be executed
  if ((_current_neighbor->active() && (_current_neighbor->level() == elem->level()) &&
       (elem_id < neighbor_id)) ||
      (_current_neighbor->level() < elem->level()))
  {
    computeInterface();
  }

  if (!_have_interface_elems &&
      (_current_elem->processor_id() != _current_neighbor->processor_id()))
  {
    // if my current neighbor is on another processor store the current element
    // ID for later communication
    _interface_elem_ids.insert(_current_elem->id());
  }
}

void
ElementLoopUserObject::post()
{
}

void
ElementLoopUserObject::join(const ElementLoopUserObject & /*y*/)
{
}

void
ElementLoopUserObject::computeElement()
{
}

void
ElementLoopUserObject::computeBoundary()
{
}

void
ElementLoopUserObject::computeInternalSide()
{
}

void
ElementLoopUserObject::computeInterface()
{
}

void
ElementLoopUserObject::meshChanged()
{
  _interface_elem_ids.clear();
  _have_interface_elems = false;
}

void
ElementLoopUserObject::caughtMooseException(MooseException & e)
{
  std::string what(e.what());
  _fe_problem.setException(what);
}
