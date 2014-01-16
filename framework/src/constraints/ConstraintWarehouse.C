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

ConstraintWarehouse::ConstraintWarehouse()
{
}

ConstraintWarehouse::~ConstraintWarehouse()
{
  for (std::map<BoundaryID, std::vector<NodeFaceConstraint *> >::iterator i = _node_face_constraints.begin(); i != _node_face_constraints.end(); ++i)
    for (std::vector<NodeFaceConstraint *>::iterator k=(i->second).begin(); k!=(i->second).end(); ++k)
      delete *k;

  for (std::map<BoundaryID, std::vector<NodeFaceConstraint *> >::iterator i = _displaced_node_face_constraints.begin(); i != _displaced_node_face_constraints.end(); ++i)
    for (std::vector<NodeFaceConstraint *>::iterator k=(i->second).begin(); k!=(i->second).end(); ++k)
      delete *k;

  for (std::map<std::string, std::vector<FaceFaceConstraint *> >::iterator i = _face_face_constraints.begin(); i != _face_face_constraints.end(); ++i)
    for (std::vector<FaceFaceConstraint *>::iterator k=(i->second).begin(); k!=(i->second).end(); ++k)
      delete *k;

  for (std::vector<NodalConstraint *>::iterator i = _nodal_constraints.begin(); i != _nodal_constraints.end(); ++i)
    delete *i;
}

void
ConstraintWarehouse::initialSetup()
{
  for (std::vector<NodalConstraint *>::const_iterator curr = _nodal_constraints.begin(); curr != _nodal_constraints.end(); ++curr)
    (*curr)->initialSetup();

  for (std::map<BoundaryID, std::vector<NodeFaceConstraint *> >::const_iterator curr = _node_face_constraints.begin(); curr != _node_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->initialSetup();

  for (std::map<BoundaryID, std::vector<NodeFaceConstraint *> >::const_iterator curr = _displaced_node_face_constraints.begin(); curr != _displaced_node_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->initialSetup();

  for (std::map<std::string, std::vector<FaceFaceConstraint *> >::const_iterator curr = _face_face_constraints.begin(); curr != _face_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->initialSetup();
}

void
ConstraintWarehouse::timestepSetup()
{
  for (std::vector<NodalConstraint *>::const_iterator curr = _nodal_constraints.begin(); curr != _nodal_constraints.end(); ++curr)
    (*curr)->timestepSetup();

  for (std::map<BoundaryID, std::vector<NodeFaceConstraint *> >::const_iterator curr = _node_face_constraints.begin(); curr != _node_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->timestepSetup();

  for (std::map<BoundaryID, std::vector<NodeFaceConstraint *> >::const_iterator curr = _displaced_node_face_constraints.begin(); curr != _displaced_node_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->timestepSetup();

  for (std::map<std::string, std::vector<FaceFaceConstraint *> >::const_iterator curr = _face_face_constraints.begin(); curr != _face_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->timestepSetup();
}

void
ConstraintWarehouse::residualSetup()
{
  for (std::vector<NodalConstraint *>::const_iterator curr = _nodal_constraints.begin(); curr != _nodal_constraints.end(); ++curr)
    (*curr)->residualSetup();

  for (std::map<BoundaryID, std::vector<NodeFaceConstraint *> >::const_iterator curr = _node_face_constraints.begin(); curr != _node_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->residualSetup();

  for (std::map<BoundaryID, std::vector<NodeFaceConstraint *> >::const_iterator curr = _displaced_node_face_constraints.begin(); curr != _displaced_node_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->residualSetup();

  for (std::map<std::string, std::vector<FaceFaceConstraint *> >::const_iterator curr = _face_face_constraints.begin(); curr != _face_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->residualSetup();
}

void
ConstraintWarehouse::jacobianSetup()
{
  for (std::vector<NodalConstraint *>::const_iterator curr = _nodal_constraints.begin(); curr != _nodal_constraints.end(); ++curr)
    (*curr)->jacobianSetup();

  for (std::map<BoundaryID, std::vector<NodeFaceConstraint *> >::const_iterator curr = _node_face_constraints.begin(); curr != _node_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->jacobianSetup();

  for (std::map<BoundaryID, std::vector<NodeFaceConstraint *> >::const_iterator curr = _displaced_node_face_constraints.begin(); curr != _displaced_node_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->jacobianSetup();

  for (std::map<std::string, std::vector<FaceFaceConstraint *> >::const_iterator curr = _face_face_constraints.begin(); curr != _face_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->jacobianSetup();
}

void
ConstraintWarehouse::addNodalConstraint(NodalConstraint * nc)
{
  _nodal_constraints.push_back(nc);
}

void
ConstraintWarehouse::addNodeFaceConstraint(unsigned int slave, unsigned int /*master*/, NodeFaceConstraint *nfc)
{
  bool displaced = nfc->parameters().have_parameter<bool>("use_displaced_mesh") && nfc->getParam<bool>("use_displaced_mesh");

  if(displaced)
    _displaced_node_face_constraints[slave].push_back(nfc);
  else
    _node_face_constraints[slave].push_back(nfc);
}

void
ConstraintWarehouse::addFaceFaceConstraint(const std::string & name, FaceFaceConstraint * ffc)
{
  _face_face_constraints[name].push_back(ffc);
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
  for (std::map<std::string, std::vector<FaceFaceConstraint *> >::const_iterator it = _face_face_constraints.begin(); it != _face_face_constraints.end(); ++it)
  {
    for (std::vector<FaceFaceConstraint *>::const_iterator jt = (*it).second.begin(); jt != (*it).second.end(); ++jt)
    {
      MooseVariable & var = (*jt)->variable();
      unique_variables.insert(var.name());
      const std::set<SubdomainID> & subdomains = var.activeSubdomains();
      subdomains_covered.insert(subdomains.begin(), subdomains.end());
    }
  }
}
