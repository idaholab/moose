//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstraintWarehouse.h"

// MOOSE includes
#include "ElemElemConstraint.h"
#include "MortarConstraint.h"
#include "RealMortarConstraintBase.h"
#include "MooseVariable.h"
#include "NodalConstraint.h"
#include "NodeFaceConstraint.h"
#include "NodeElemConstraint.h"

ConstraintWarehouse::ConstraintWarehouse() : MooseObjectWarehouse<Constraint>(/*threaded=*/false) {}

void
ConstraintWarehouse::addObject(std::shared_ptr<Constraint> object,
                               THREAD_ID /*tid = 0*/,
                               bool /*recurse = true*/)
{
  // Adds to the storage of _all_objects
  MooseObjectWarehouse<Constraint>::addObject(object);

  // Cast the the possible Contraint types
  std::shared_ptr<NodeFaceConstraint> nfc = std::dynamic_pointer_cast<NodeFaceConstraint>(object);
  std::shared_ptr<MortarConstraint> ffc = std::dynamic_pointer_cast<MortarConstraint>(object);
  std::shared_ptr<RealMortarConstraintBase> mc =
      std::dynamic_pointer_cast<RealMortarConstraintBase>(object);
  std::shared_ptr<NodalConstraint> nc = std::dynamic_pointer_cast<NodalConstraint>(object);
  std::shared_ptr<ElemElemConstraint> ec = std::dynamic_pointer_cast<ElemElemConstraint>(object);
  std::shared_ptr<NodeElemConstraint> nec = std::dynamic_pointer_cast<NodeElemConstraint>(object);

  // NodeFaceConstraint
  if (nfc)
  {
    MooseMesh & mesh = nfc->getParam<FEProblemBase *>("_fe_problem_base")->mesh();
    unsigned int slave = mesh.getBoundaryID(nfc->getParam<BoundaryName>("slave"));
    bool displaced = nfc->parameters().have_parameter<bool>("use_displaced_mesh") &&
                     nfc->getParam<bool>("use_displaced_mesh");

    if (displaced)
      _displaced_node_face_constraints[slave].addObject(nfc);
    else
      _node_face_constraints[slave].addObject(nfc);
  }

  // MortarConstraint
  else if (ffc)
  {
    const std::string & interface = ffc->getParam<std::string>("interface");
    _mortar_constraints[interface].addObject(ffc);
  }

  // RealMortarConstraint
  else if (mc)
    _real_mortar_constraints.addObject(mc);

  // ElemElemConstraint
  else if (ec)
  {
    bool displaced = ec->parameters().have_parameter<bool>("use_displaced_mesh") &&
                     ec->getParam<bool>("use_displaced_mesh");
    const InterfaceID interface_id = ec->getInterfaceID();

    if (displaced)
      _displaced_element_constraints[interface_id].addObject(ec);
    else
      _element_constraints[interface_id].addObject(ec);
  }

  // NodeElemConstraint
  else if (nec)
  {
    MooseMesh & mesh = nec->getParam<FEProblemBase *>("_fe_problem_base")->mesh();
    SubdomainID slave = mesh.getSubdomainID(nec->getParam<SubdomainName>("slave"));
    SubdomainID master = mesh.getSubdomainID(nec->getParam<SubdomainName>("master"));
    bool displaced = nec->parameters().have_parameter<bool>("use_displaced_mesh") &&
                     nec->getParam<bool>("use_displaced_mesh");

    if (displaced)
      _displaced_node_elem_constraints[std::make_pair(slave, master)].addObject(nec);
    else
      _node_elem_constraints[std::make_pair(slave, master)].addObject(nec);
  }

  // NodalConstraint
  else if (nc)
    _nodal_constraints.addObject(nc);

  else
    mooseError("Unknown type of Constraint object");
}

const std::vector<std::shared_ptr<NodalConstraint>> &
ConstraintWarehouse::getActiveNodalConstraints() const
{
  return _nodal_constraints.getActiveObjects();
}

const std::vector<std::shared_ptr<NodeFaceConstraint>> &
ConstraintWarehouse::getActiveNodeFaceConstraints(BoundaryID boundary_id, bool displaced) const
{
  std::map<BoundaryID, MooseObjectWarehouse<NodeFaceConstraint>>::const_iterator it, end_it;

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

  mooseAssert(it != end_it,
              "Unable to locate storage for NodeFaceConstraint objects for the given boundary id: "
                  << boundary_id);
  return it->second.getActiveObjects();
}

const std::vector<std::shared_ptr<MortarConstraint>> &
ConstraintWarehouse::getActiveMortarConstraints(const std::string & interface) const
{
  std::map<std::string, MooseObjectWarehouse<MortarConstraint>>::const_iterator it =
      _mortar_constraints.find(interface);
  mooseAssert(it != _mortar_constraints.end(),
              "Unable to locate storage for MortarConstraint objects for the given interface: "
                  << interface);
  return it->second.getActiveObjects();
}

const std::vector<std::shared_ptr<RealMortarConstraintBase>> &
ConstraintWarehouse::getActiveRealMortarConstraints() const
{
  return _real_mortar_constraints.getActiveObjects();
}

const std::vector<std::shared_ptr<ElemElemConstraint>> &
ConstraintWarehouse::getActiveElemElemConstraints(const InterfaceID interface_id,
                                                  bool displaced) const
{
  std::map<unsigned int, MooseObjectWarehouse<ElemElemConstraint>>::const_iterator it, end_it;

  if (displaced)
  {
    it = _displaced_element_constraints.find(interface_id);
    end_it = _displaced_element_constraints.end();
  }

  else
  {
    it = _element_constraints.find(interface_id);
    end_it = _element_constraints.end();
  }

  mooseAssert(it != end_it,
              "Unable to locate storage for ElemElemConstraint objects for the given interface id: "
                  << interface_id);
  return it->second.getActiveObjects();
}

const std::vector<std::shared_ptr<NodeElemConstraint>> &
ConstraintWarehouse::getActiveNodeElemConstraints(SubdomainID slave_id,
                                                  SubdomainID master_id,
                                                  bool displaced) const
{
  std::map<std::pair<SubdomainID, SubdomainID>,
           MooseObjectWarehouse<NodeElemConstraint>>::const_iterator it,
      end_it;

  if (displaced)
  {
    it = _displaced_node_elem_constraints.find(std::make_pair(slave_id, master_id));
    end_it = _displaced_node_elem_constraints.end();
  }
  else
  {
    it = _node_elem_constraints.find(std::make_pair(slave_id, master_id));
    end_it = _node_elem_constraints.end();
  }

  mooseAssert(it != end_it,
              "Unable to locate storage for NodeElemConstraint objects for the given slave and "
              "master id pair: ["
                  << slave_id << ", " << master_id << "]");
  return it->second.getActiveObjects();
}

bool
ConstraintWarehouse::hasActiveNodalConstraints() const
{
  return _nodal_constraints.hasActiveObjects();
}

bool
ConstraintWarehouse::hasActiveMortarConstraints(const std::string & interface) const
{
  std::map<std::string, MooseObjectWarehouse<MortarConstraint>>::const_iterator it =
      _mortar_constraints.find(interface);
  return (it != _mortar_constraints.end() && it->second.hasActiveObjects());
}

bool
ConstraintWarehouse::hasActiveElemElemConstraints(const InterfaceID interface_id,
                                                  bool displaced) const
{
  std::map<unsigned int, MooseObjectWarehouse<ElemElemConstraint>>::const_iterator it, end_it;

  if (displaced)
  {
    it = _displaced_element_constraints.find(interface_id);
    end_it = _displaced_element_constraints.end();
  }

  else
  {
    it = _element_constraints.find(interface_id);
    end_it = _element_constraints.end();
  }

  return (it != end_it && it->second.hasActiveObjects());
}

bool
ConstraintWarehouse::hasActiveNodeFaceConstraints(BoundaryID boundary_id, bool displaced) const
{
  std::map<BoundaryID, MooseObjectWarehouse<NodeFaceConstraint>>::const_iterator it, end_it;

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

  return (it != end_it && it->second.hasActiveObjects());
}

bool
ConstraintWarehouse::hasActiveNodeElemConstraints(SubdomainID slave_id,
                                                  SubdomainID master_id,
                                                  bool displaced) const
{
  std::map<std::pair<SubdomainID, SubdomainID>,
           MooseObjectWarehouse<NodeElemConstraint>>::const_iterator it,
      end_it;

  if (displaced)
  {
    it = _displaced_node_elem_constraints.find(std::make_pair(slave_id, master_id));
    end_it = _displaced_node_elem_constraints.end();
  }

  else
  {
    it = _node_elem_constraints.find(std::make_pair(slave_id, master_id));
    end_it = _node_elem_constraints.end();
  }

  return (it != end_it && it->second.hasActiveObjects());
}

void ConstraintWarehouse::updateActive(THREAD_ID /*tid*/)
{
  MooseObjectWarehouse<Constraint>::updateActive();
  _nodal_constraints.updateActive();

  for (auto & it : _node_face_constraints)
    it.second.updateActive();

  for (auto & it : _displaced_node_face_constraints)
    it.second.updateActive();

  // FIXME: We call updateActive() on the NodeFaceConstraints again?
  for (auto & it : _node_face_constraints)
    it.second.updateActive();

  for (auto & it : _element_constraints)
    it.second.updateActive();

  for (auto & it : _node_elem_constraints)
    it.second.updateActive();
}

void
ConstraintWarehouse::subdomainsCovered(std::set<SubdomainID> & subdomains_covered,
                                       std::set<std::string> & unique_variables,
                                       THREAD_ID /*tid=0*/) const
{
  for (const auto & it : _mortar_constraints)
  {
    const auto & objects = it.second.getActiveObjects();
    for (const auto & ffc : objects)
    {
      MooseVariableFEBase & var = ffc->variable();
      unique_variables.insert(var.name());
      const std::set<SubdomainID> & subdomains = var.activeSubdomains();
      subdomains_covered.insert(subdomains.begin(), subdomains.end());
    }
  }
}

void
ConstraintWarehouse::residualEnd(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  for (const auto & object : _active_objects[tid])
    object->residualEnd();
}
