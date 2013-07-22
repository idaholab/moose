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

#ifndef SPLITWAREHOUSE_H
#define SPLITWAREHOUSE_H

#include <vector>
#include <map>
#include <set>

#include "MooseTypes.h"

class Split;

/**
 * Holds splits and provides some services
 */
class SplitWarehouse
{
public:
  SplitWarehouse();
  virtual ~SplitWarehouse();

  /**
   * Get list of all splits
   * @return The list of all splits
   */
  const std::vector<Split *> & all();

  /**
   * Add a split
   * @param split Split being added
   */
  void   addSplit(const std::string& name, Split* split);
  Split *getSplit(const std::string& name);

protected:
  /// all splits
  std::map<std::string, Split *> _all_splits;
};

#endif // SPLITWAREHOUSE_H
