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

// MOOSE includes
#include "ConstraintWarehouse.h"
#include "NodalConstraint.h"
#include "NodeFaceConstraint.h"
#include "FaceFaceConstraint.h"
#include "ElemElemConstraint.h"

ConstraintWarehouse::ConstraintWarehouse() :
    MooseObjectWarehouse<Constraint>(/*threaded=*/false)
{
}


void
ConstraintWarehouse::addObject(MooseSharedPointer<Constraint> object, THREAD_ID /*tid = 0*/)
{
  // Adds to the storage of _all_objects
  MooseObjectWarehouse<Constraint>::addObject(object);

  // Cast the the possible Contraint types
  MooseSharedPointer<NodeFaceConstraint> nfc = MooseSharedNamespace::dynamic_pointer_cast<NodeFaceConstraint>(object);
  MooseSharedPointer<FaceFaceConstraint> ffc = MooseSharedNamespace::dynamic_pointer_cast<FaceFaceConstraint>(object);
  MooseSharedPointer<NodalConstraint>    nc =  MooseSharedNamespace::dynamic_pointer_cast<NodalConstraint>(object);
  MooseSharedPointer<ElemElemConstraint>  ec = MooseSharedNamespace::dynamic_pointer_cast<ElemElemConstraint>(object);

  // NodeFaceConstraint
  if (nfc)
  {
    MooseMesh & mesh = nfc->getParam<FEProblem *>("_fe_problem")->mesh();
    unsigned int slave = mesh.getBoundaryID(nfc->getParam<BoundaryName>("slave"));
    bool displaced = nfc->parameters().have_parameter<bool>("use_displaced_mesh") && nfc->getParam<bool>("use_displaced_mesh");

    if (displaced)
      _displaced_node_face_constraints[slave].addObject(nfc);
    else
      _node_face_constraints[slave].addObject(nfc);
  }

  // FaceFaceConstraint
  else if (ffc)
  {
    const std::string & interface = ffc->getParam<std::string>("interface");
    _face_face_constraints[interface].addObject(ffc);
  }

  // ElemElemConstraint
  else if (ec)
  {
    const InterfaceID interface_id = ec->getParam<InterfaceID>("interface_id");
    _element_constraints[interface_id].addObject(ec);
  }

  // NodalConstraint
  else if (nc)
    _nodal_constraints.addObject(nc);

  else
    mooseError("Unknown type of Constraint object");
}


const std::vector<MooseSharedPointer<NodalConstraint> > &
ConstraintWarehouse::getActiveNodalConstraints() const
{
  return _nodal_constraints.getActiveObjects();
}


const std::vector<MooseSharedPointer<NodeFaceConstraint> > &
ConstraintWarehouse::getActiveNodeFaceConstraints(BoundaryID boundary_id, bool displaced)
{
  std::map<BoundaryID, MooseObjectWarehouse<NodeFaceConstraint> >::const_iterator it, end_it;

  if (displaced)
  {
    it = _displaced_node_face_constraints.find(boundary_id);
    end_it = _displaced_node_face_constraints.end();
  }

  else
  {
    it = _node_face_constraints.find(boundary_id);
    end_it = _node_face_constraints.end();
  }

  mooseAssert(it != end_it, "Unable to locate storage for NodeFaceConstraint objects for the given boundary id: " << boundary_id);
  return it->second.getActiveObjects();
}


const std::vector<MooseSharedPointer<FaceFaceConstraint> > &
ConstraintWarehouse::getActiveFaceFaceConstraints(const std::string & interface) const
{
  std::map<std::string, MooseObjectWarehouse<FaceFaceConstraint> >::const_iterator it = _face_face_constraints.find(interface);
  mooseAssert(it != _face_face_constraints.end(), "Unable to locate storage for FaceFaceConstraint objects for the given interface: " << interface);
  return it->second.getActiveObjects();
}

const std::vector<MooseSharedPointer<ElemElemConstraint> > &
ConstraintWarehouse::getActiveElemElemConstraints(const InterfaceID interface_id) const
{
  std::map<unsigned int, MooseObjectWarehouse<ElemElemConstraint> >::const_iterator it = _element_constraints.find(interface_id);
  mooseAssert(it != _element_constraints.end(), "Unable to locate storage for ElemElemConstraint objects for the given interface id: " << interface_id);
  return it->second.getActiveObjects();
}

bool
ConstraintWarehouse::hasActiveNodalConstraints() const
{
  return _nodal_constraints.hasActiveObjects();
}


bool
ConstraintWarehouse::hasActiveFaceFaceConstraints(const std::string & interface) const
{
  std::map<std::string, MooseObjectWarehouse<FaceFaceConstraint> >::const_iterator it = _face_face_constraints.find(interface);
  return (it != _face_face_constraints.end() && it->second.hasActiveObjects());
}

bool
ConstraintWarehouse::hasActiveElemElemConstraints(const InterfaceID interface_id) const
{
  std::map<unsigned int, MooseObjectWarehouse<ElemElemConstraint> >::const_iterator it = _element_constraints.find(interface_id);
  return (it != _element_constraints.end() && it->second.hasActiveObjects());
}


bool
ConstraintWarehouse::hasActiveNodeFaceConstraints(BoundaryID boundary_id, bool displaced) const
{
  std::map<BoundaryID, MooseObjectWarehouse<NodeFaceConstraint> >::const_iterator it, end_it;

  if (displaced)
  {
    it = _displaced_node_face_constraints.find(boundary_id);
    end_it = _displaced_node_face_constraints.end();
  }

  else
  {
    it = _node_face_constraints.find(boundary_id);
    end_it = _node_face_constraints.end();
  }

  return(it != end_it && it->second.hasActiveObjects());
}


void
ConstraintWarehouse::updateActive(THREAD_ID /*tid*/)
{
  MooseObjectWarehouse<Constraint>::updateActive();
  _nodal_constraints.updateActive();

  {
    std::map<BoundaryID, MooseObjectWarehouse<NodeFaceConstraint> >::iterator it;
    for (it = _node_face_constraints.begin(); it != _node_face_constraints.end(); ++it)
      it->second.updateActive();
    for (it = _displaced_node_face_constraints.begin(); it != _displaced_node_face_constraints.end(); ++it)
      it->second.updateActive();
  }

  {
    std::map<std::string, MooseObjectWarehouse<FaceFaceConstraint> >::iterator it;
    for (std::map<BoundaryID, MooseObjectWarehouse<NodeFaceConstraint> >::iterator it = _node_face_constraints.begin(); it != _node_face_constraints.end(); ++it)
      it->second.updateActive();
  }

  {
    std::map<unsigned int, MooseObjectWarehouse<ElemElemConstraint> >::iterator it;
    for (it = _element_constraints.begin(); it != _element_constraints.end(); ++it)
      it->second.updateActive();
  }
}


void
ConstraintWarehouse::subdomainsCovered(std::set<SubdomainID> & subdomains_covered, std::set<std::string> & unique_variables, THREAD_ID/*tid=0*/) const
{
  std::map<std::string, MooseObjectWarehouse<FaceFaceConstraint> >::const_iterator it;

  for (it = _face_face_constraints.begin(); it != _face_face_constraints.end(); ++it)
  {
    const std::vector<MooseSharedPointer<FaceFaceConstraint> > & objects = it->second.getActiveObjects();
    for (std::vector<MooseSharedPointer<FaceFaceConstraint> >::const_iterator jt = objects.begin(); jt != objects.end(); ++jt)
    {
      MooseVariable & var = (*jt)->variable();
      unique_variables.insert(var.name());
      const std::set<SubdomainID> & subdomains = var.activeSubdomains();
      subdomains_covered.insert(subdomains.begin(), subdomains.end());
    }
  }
}
