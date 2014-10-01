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

SplitWarehouse::SplitWarehouse() :
    Warehouse<Split>()
{
}

SplitWarehouse::~SplitWarehouse()
{
}

void
SplitWarehouse::addSplit(const std::string& name, MooseSharedPointer<Split> & split)
{
  _all_objects.push_back(split.get());
  _all_splits_by_name.insert(std::make_pair(name, split));
}

Split*
SplitWarehouse::getSplit(const std::string& name)
{
  std::map<std::string, MooseSharedPointer<Split> >::iterator it = _all_splits_by_name.find(name);

  if (it == _all_splits_by_name.end())
    mooseError("No split named '" << name << "'");

  return it->second.get();
}
