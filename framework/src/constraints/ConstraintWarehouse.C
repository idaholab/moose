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

ConstraintWarehouse::ConstraintWarehouse()
{
}

ConstraintWarehouse::~ConstraintWarehouse()
{
  for (std::map<unsigned int, std::vector<NodeFaceConstraint *> >::iterator i = _node_face_constraints.begin(); i != _node_face_constraints.end(); ++i)
    for (std::vector<NodeFaceConstraint *>::iterator k=(i->second).begin(); k!=(i->second).end(); ++k)
      delete *k;

  for (std::map<unsigned int, std::vector<NodeFaceConstraint *> >::iterator i = _displaced_node_face_constraints.begin(); i != _displaced_node_face_constraints.end(); ++i)
    for (std::vector<NodeFaceConstraint *>::iterator k=(i->second).begin(); k!=(i->second).end(); ++k)
      delete *k;
}

void
ConstraintWarehouse::initialSetup()
{
  for (std::map<unsigned int, std::vector<NodeFaceConstraint *> >::const_iterator curr = _node_face_constraints.begin(); curr != _node_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->initialSetup();

  for (std::map<unsigned int, std::vector<NodeFaceConstraint *> >::const_iterator curr = _displaced_node_face_constraints.begin(); curr != _displaced_node_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->initialSetup();
}

void
ConstraintWarehouse::timestepSetup()
{
  for (std::map<unsigned int, std::vector<NodeFaceConstraint *> >::const_iterator curr = _node_face_constraints.begin(); curr != _node_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->timestepSetup();

  for (std::map<unsigned int, std::vector<NodeFaceConstraint *> >::const_iterator curr = _displaced_node_face_constraints.begin(); curr != _displaced_node_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->timestepSetup();
}

void
ConstraintWarehouse::residualSetup()
{
  for (std::map<unsigned int, std::vector<NodeFaceConstraint *> >::const_iterator curr = _node_face_constraints.begin(); curr != _node_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->residualSetup();

  for (std::map<unsigned int, std::vector<NodeFaceConstraint *> >::const_iterator curr = _displaced_node_face_constraints.begin(); curr != _displaced_node_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->residualSetup();
}

void
ConstraintWarehouse::jacobianSetup()
{
  for (std::map<unsigned int, std::vector<NodeFaceConstraint *> >::const_iterator curr = _node_face_constraints.begin(); curr != _node_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->jacobianSetup();

  for (std::map<unsigned int, std::vector<NodeFaceConstraint *> >::const_iterator curr = _displaced_node_face_constraints.begin(); curr != _displaced_node_face_constraints.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->jacobianSetup();
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

std::vector<NodeFaceConstraint *> &
ConstraintWarehouse::getNodeFaceConstraints(unsigned int boundary_id)
{
  return _node_face_constraints[boundary_id];
}

std::vector<NodeFaceConstraint *> &
ConstraintWarehouse::getDisplacedNodeFaceConstraints(unsigned int boundary_id)
{
  return _displaced_node_face_constraints[boundary_id];
}
