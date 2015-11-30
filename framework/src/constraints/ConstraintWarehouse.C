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

#include "ConstraintWarehouse.h"
#include "NodalConstraint.h"
#include "NodeFaceConstraint.h"
#include "FaceFaceConstraint.h"
#include "FEProblem.h"

ConstraintWarehouse::ConstraintWarehouse() :
    MooseObjectWarehouse<Constraint>()
{
}

ConstraintWarehouse::~ConstraintWarehouse()
{
}

void
ConstraintWarehouse::addObject(MooseSharedPointer<Constraint> constraint, THREAD_ID /*tid = 0*/)
{

  // Adds the constraint to the complete list of objects
  MooseObjectWarehouse::addObject(constraint);

  // Cast the Constraint object to the various types stored in this wareouse
  MooseSharedPointer<NodalConstraint> nodal =  MooseSharedNamespace::dynamic_pointer_cast<NodalConstraint>(constraint);
  MooseSharedPointer<NodeFaceConstraint> node_face = MooseSharedNamespace::dynamic_pointer_cast<NodeFaceConstraint>(constraint);
  MooseSharedPointer<FaceFaceConstraint> face_face = MooseSharedNamespace::dynamic_pointer_cast<FaceFaceConstraint>(constraint);

  // NodeFaceConstraint
  if (node_face)
  {
    MooseMesh & mesh = constraint->getParam<FEProblem *>("_fe_problem")->mesh();
    unsigned int slave = mesh.getBoundaryID(constraint->getParam<BoundaryName>("slave"));
    unsigned int master = mesh.getBoundaryID(constraint->getParam<BoundaryName>("master"));

    bool displaced = node_face->isParamValid("use_displaced_mesh") && node_face->getParam<bool>("use_displaced_mesh");
    if (displaced)
      _displaced_node_face_constraints[slave].addObject(node_face);
    else
      _node_face_constraints[slave].addObject(node_face);
  }

  // FaceFaceConstraint
  else if (face_face)
  {
    const std::string & interface = face_face->getParam<std::string>("interface");
    _face_face_constraints[interface].addObject(face_face);
  }

  // NodalConstraint
  else if (nodal)
    _nodal_constraints.addObject(nodal);

  else
    mooseError("Unknown type of Constraint object");

}


bool
ConstraintWarehouse::hasActiveNodalConstraints() const
{
  return _nodal_constraints.hasActiveObjects();
}


bool
ConstraintWarehouse::hasActiveNodeFaceConstraints(BoundaryID boundary_id, bool displaced /*= false*/) const
{
  std::map<BoundaryID, MooseObjectStorage<NodeFaceConstraint> >::const_iterator iter, end;
  if (displaced)
  {
    iter = _displaced_node_face_constraints.find(boundary_id);
    end = _displaced_node_face_constraints.end();
  }

  else
  {
    iter = _node_face_constraints.find(boundary_id);
    end = _node_face_constraints.end();
  }
  return (iter != end && iter->second.hasActiveObjects());
}


bool
ConstraintWarehouse::hasActiveFaceFaceConstraints(const std::string & name) const
{
  std::map<std::string, MooseObjectStorage<FaceFaceConstraint> >::const_iterator iter = _face_face_constraints.find(name);
  return (iter != _face_face_constraints.end() && iter->second.hasActiveObjects());
}


const std::vector<MooseSharedPointer<NodalConstraint> > &
ConstraintWarehouse::getActiveNodalConstraints() const
{
  return _nodal_constraints.getActiveObjects();
}


const std::vector<MooseSharedPointer<NodeFaceConstraint> > &
ConstraintWarehouse::getActiveNodeFaceConstraints(BoundaryID boundary_id, bool displaced /*=false*/) const
{
  std::map<BoundaryID, MooseObjectStorage<NodeFaceConstraint> >::const_iterator iter;
  if (displaced)
    iter = _displaced_node_face_constraints.find(boundary_id);
  else
    iter = _node_face_constraints.find(boundary_id);

  return iter->second.getActiveObjects();
}


const std::vector<MooseSharedPointer<FaceFaceConstraint> > &
ConstraintWarehouse::getActiveFaceFaceConstraints(const std::string & name) const
{
  std::map<std::string, MooseObjectStorage<FaceFaceConstraint> >::const_iterator iter = _face_face_constraints.find(name);
  return iter->second.getActiveObjects();
}


void
ConstraintWarehouse::subdomainsCovered(std::set<SubdomainID> & subdomains_covered, std::set<std::string> & unique_variables) const
{
  std::map<std::string, MooseObjectStorage<FaceFaceConstraint> >::const_iterator it;
  std::vector<MooseSharedPointer<FaceFaceConstraint> >::const_iterator jt;

  for (it = _face_face_constraints.begin(); it != _face_face_constraints.end(); ++it)
  {
    const std::vector<MooseSharedPointer<FaceFaceConstraint> > objects = it->second.getActiveObjects();
    for (jt = objects.begin(); jt != objects.end(); ++jt)
    {
      MooseVariable & var = (*jt)->variable();
      unique_variables.insert(var.name());
      const std::set<SubdomainID> & subdomains = var.activeSubdomains();
      subdomains_covered.insert(subdomains.begin(), subdomains.end());
    }
  }
}


void
ConstraintWarehouse::updateActive(THREAD_ID /*tid*/)
{
  MooseObjectWarehouse::updateActive();
  _nodal_constraints.updateActive();

  {
    std::map<BoundaryID, MooseObjectStorage<NodeFaceConstraint> >::iterator it;
    for (it = _node_face_constraints.begin(); it != _node_face_constraints.end(); ++it)
      it->second.updateActive();
    for (it = _displaced_node_face_constraints.begin(); it != _displaced_node_face_constraints.end(); ++it)
      it->second.updateActive();
  }

  {
    std::map<std::string, MooseObjectStorage<FaceFaceConstraint> >::iterator it;
    for (it = _face_face_constraints.begin(); it != _face_face_constraints.end(); ++it)
      it->second.updateActive();
  }
}
