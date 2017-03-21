/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "EFANode.h"

#include <sstream>

EFANode::EFANode(unsigned int nid, N_CATEGORY ncat, EFANode * nparent)
  : _category(ncat), _id(nid), _parent(nparent)
{
}

std::string
EFANode::idCatString()
{
  std::ostringstream s;
  s << _id;
  if (_category == N_CATEGORY_EMBEDDED)
    s << "e";
  else if (_category == N_CATEGORY_TEMP)
    s << "t";
  else
    s << " ";
  return s.str();
}

unsigned int
EFANode::id() const
{
  return _id;
}

EFANode::N_CATEGORY
EFANode::category() const
{
  return _category;
}

EFANode *
EFANode::parent() const
{
  return _parent;
}

void
EFANode::removeParent()
{
  _parent = NULL;
}
