//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
  else if (_category == N_CATEGORY_EMBEDDED_PERMANENT)
    s << "ep";
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

void
EFANode::setCategory(EFANode::N_CATEGORY category)
{
  _category = category;
}
