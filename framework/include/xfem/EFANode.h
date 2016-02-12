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

#include <string>

enum N_CATEGORY
{
  N_CATEGORY_PERMANENT,
  N_CATEGORY_TEMP,
  N_CATEGORY_EMBEDDED,
  N_CATEGORY_LOCAL_INDEX
};

class EFANode
{
public:

  EFANode(unsigned int nid, N_CATEGORY ncat, EFANode* nparent=NULL);

private:

  N_CATEGORY _category;
  unsigned int _id;
  EFANode* _parent;

public:

  std::string idCatString();
  unsigned int id() const;
  N_CATEGORY category() const;
  EFANode* parent() const;
  void removeParent();
};


#endif
