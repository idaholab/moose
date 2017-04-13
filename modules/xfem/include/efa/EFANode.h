/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef EFANODE_H
#define EFANODE_H

#include <string>

class EFANode
{
public:
  enum N_CATEGORY
  {
    N_CATEGORY_PERMANENT,
    N_CATEGORY_TEMP,
    N_CATEGORY_EMBEDDED,
    N_CATEGORY_LOCAL_INDEX
  };

  EFANode(unsigned int nid, N_CATEGORY ncat, EFANode * nparent = NULL);

private:
  N_CATEGORY _category;
  unsigned int _id;
  EFANode * _parent;

public:
  std::string idCatString();
  unsigned int id() const;
  N_CATEGORY category() const;
  EFANode * parent() const;
  void removeParent();
};

#endif
