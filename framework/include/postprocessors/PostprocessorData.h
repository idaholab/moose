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

#ifndef POSTPROCESSORDATA_H
#define POSTPROCESSORDATA_H

#include <map>
//MOOSE includes
#include "Moose.h"

class PostprocessorData
{
public:
  PostprocessorData();

  void init(const std::string & name);

  bool hasPostprocessor(const std::string & name);

  PostprocessorValue & getPostprocessorValue(const std::string & name);

  void storeValue(const std::string & name, PostprocessorValue value);

  /**
   * Get the map of names -> postprocessor values. Exposed for error checking.
   */
  const std::map<std::string, PostprocessorValue> & values() const { return _values; }

protected:
  std::map<std::string, PostprocessorValue> _values;
};

#endif //POSTPROCESSORDATA_H
