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

#include "SplitWarehouse.h"
#include "Split.h"

SplitWarehouse::SplitWarehouse()
{
}

SplitWarehouse::~SplitWarehouse()
{
  for (std::map<std::string, Split *>::iterator i = _all_splits.begin(); i != _all_splits.end(); ++i)
    delete i->second;
}

void
SplitWarehouse::addSplit(const std::string& name, Split* split)
{
  if (!split) {
    std::ostringstream err;
    err << "Attempted addition of a NULL split";
    mooseError(err.str());
  }
  std::pair<std::string,Split*> pair(name,split);
  _all_splits.insert(pair);
}

Split*
SplitWarehouse::getSplit(const std::string& name)
{
  Split *split = NULL;
  std::map<std::string,Split*>::iterator it = _all_splits.find(name);
  if (it != _all_splits.end()) {
    split = it->second;
  } else {
    std::ostringstream err;
    err << "No split named '" << name << "'";
    mooseError(err.str());
  }
  return split;
}


