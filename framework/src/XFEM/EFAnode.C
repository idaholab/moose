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

#include "EFAnode.h"

EFAnode::EFAnode(unsigned int nid, N_CATEGORY ncat, EFAnode* nparent):
  _category(ncat),
  _id(nid),
  _parent(nparent)
{};

std::string
EFAnode::id_cat_str()
{
  std::ostringstream s;
  s << _id;
  if (_category == N_CATEGORY_EMBEDDED)
    s<<"e";
  else if (_category == N_CATEGORY_TEMP)
    s<<"t";
  else
    s<<" ";
  return s.str();
}

unsigned int
EFAnode::id() const
{
  return _id;
}

N_CATEGORY
EFAnode::category() const
{
  return _category;
}

EFAnode*
EFAnode::parent() const
{
  return _parent;
}

void
EFAnode::remove_parent()
{
  _parent = NULL;
}
