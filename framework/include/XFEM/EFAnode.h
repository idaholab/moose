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

#ifndef EFANODE_H
#define EFANODE_H

#include <cstddef>
#include <iostream>
#include <sstream>
#include <vector>

enum N_CATEGORY
{
  N_CATEGORY_PERMANENT,
  N_CATEGORY_TEMP,
  N_CATEGORY_EMBEDDED,
  N_CATEGORY_LOCAL_INDEX
};

class EFAnode
{
public:

  EFAnode(unsigned int nid, N_CATEGORY ncat, EFAnode* nparent=NULL);

private:

  N_CATEGORY _category;
  unsigned int _id;
  EFAnode* _parent;

public:

  std::string id_cat_str();
  unsigned int id() const;
  N_CATEGORY category() const;
  EFAnode* parent() const;
  void remove_parent();
};

#endif
