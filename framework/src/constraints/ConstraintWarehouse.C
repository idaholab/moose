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

ConstraintWarehouse::ConstraintWarehouse() :
    Warehouse<Constraint>()
{
}

ConstraintWarehouse::~ConstraintWarehouse()
{
}

void
ConstraintWarehouse::initialSetup()
{
  for (NodalConstraintIter curr = _nodal_constraints.begin(); curr != _nodal_constraints.end(); ++curr)
    (*curr)->initialSetup();

  for (NodeFaceIter curr = _node_face_constraints.begin(); curr != _node_face_constraints.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->initialSetup();

  for (NodeFaceIter curr = _displaced_node_face_constraints.begin(); curr != _displaced_node_face_constraints.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->initialSetup();

  for (FaceFaceIter curr = _face_face_constraints.begin(); curr != _face_face_constraints.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->initialSetup();
}

void
ConstraintWarehouse::timestepSetup()
{
  for (NodalConstraintIter curr = _nodal_constraints.begin(); curr != _nodal_constraints.end(); ++curr)
    (*curr)->timestepSetup();

  for (NodeFaceIter curr = _node_face_constraints.begin(); curr != _node_face_constraints.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->timestepSetup();

  for (NodeFaceIter curr = _displaced_node_face_constraints.begin(); curr != _displaced_node_face_constraints.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->timestepSetup();

  for (FaceFaceIter curr = _face_face_constraints.begin(); curr != _face_face_constraints.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->timestepSetup();
}

void
ConstraintWarehouse::residualSetup()
{
  for (NodalConstraintIter curr = _nodal_constraints.begin(); curr != _nodal_constraints.end(); ++curr)
    (*curr)->residualSetup();

  for (NodeFaceIter curr = _node_face_constraints.begin(); curr != _node_face_constraints.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->residualSetup();

  for (NodeFaceIter curr = _displaced_node_face_constraints.begin(); curr != _displaced_node_face_constraints.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->residualSetup();

  for (FaceFaceIter curr = _face_face_constraints.begin(); curr != _face_face_constraints.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->residualSetup();
}

void
ConstraintWarehouse::jacobianSetup()
{
  for (NodalConstraintIter curr = _nodal_constraints.begin(); curr != _nodal_constraints.end(); ++curr)
    (*curr)->jacobianSetup();

  for (NodeFaceIter curr = _node_face_constraints.begin(); curr != _node_face_constraints.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->jacobianSetup();

  for (NodeFaceIter curr = _displaced_node_face_constraints.begin(); curr != _displaced_node_face_constraints.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->jacobianSetup();

  for (FaceFaceIter curr = _face_face_constraints.begin(); curr != _face_face_constraints.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->jacobianSetup();
}

void
ConstraintWarehouse::addNodalConstraint(MooseSharedPointer<NodalConstraint> nc)
{
  _all_objects.push_back(nc.get());
  _all_ptrs.push_back(nc);
  _nodal_constraints.push_back(nc.get());
}

void
ConstraintWarehouse::addNodeFaceConstraint(unsigned int slave, unsigned int /*master*/, MooseSharedPointer<NodeFaceConstraint> nfc)
{
  _all_objects.push_back(nfc.get());
  _all_ptrs.push_back(nfc);

  bool displaced = nfc->parameters().have_parameter<bool>("use_displaced_mesh") && nfc->getParam<bool>("use_displaced_mesh");

  if (displaced)
    _displaced_node_face_constraints[slave].push_back(nfc.get());
  else
    _node_face_constraints[slave].push_back(nfc.get());
}

void
ConstraintWarehouse::addFaceFaceConstraint(const std::string & name, MooseSharedPointer<FaceFaceConstraint> ffc)
{
  _all_objects.push_back(ffc.get());
  _all_ptrs.push_back(ffc);
  _face_face_constraints[name].push_back(ffc.get());
}

std::vector<NodalConstraint *> &
ConstraintWarehouse::getNodalConstraints()
{
  return _nodal_constraints;
}

std::vector<NodeFaceConstraint *> &
ConstraintWarehouse::getNodeFaceConstraints(BoundaryID boundary_id)
{
  return _node_face_constraints[boundary_id];
}

std::vector<NodeFaceConstraint *> &
ConstraintWarehouse::getDisplacedNodeFaceConstraints(BoundaryID boundary_id)
{
  return _displaced_node_face_constraints[boundary_id];
}

std::vector<FaceFaceConstraint *> &
ConstraintWarehouse::getFaceFaceConstraints(const std::string & name)
{
  return _face_face_constraints[name];
}

void
ConstraintWarehouse::subdomainsCovered(std::set<SubdomainID> & subdomains_covered, std::set<std::string> & unique_variables) const
{
  for (FaceFaceIter it = _face_face_constraints.begin(); it != _face_face_constraints.end(); ++it)
  {
    for (std::vector<FaceFaceConstraint *>::const_iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
    {
      MooseVariable & var = (*jt)->variable();
      unique_variables.insert(var.name());
      const std::set<SubdomainID> & subdomains = var.activeSubdomains();
      subdomains_covered.insert(subdomains.begin(), subdomains.end());
    }
  }
}
