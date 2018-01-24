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

// MOOSE includes
#include "MooseTypes.h"
#include "Restartable.h"

#include <map>

class FEProblemBase;

class PostprocessorData : public Restartable
{
public:
  /**
   * Class constructor
   */
  PostprocessorData(FEProblemBase & fe_problem);

  /**
   * Initialization method, sets the current and old value to 0.0 for this
   * postprocessor
   */
  void init(const std::string & name);

  /**
   * Returns a true value if the Postprocessor exists
   */
  bool hasPostprocessor(const std::string & name);

  /**
   * Return the value for the Postprocessor
   */
  PostprocessorValue & getPostprocessorValue(const PostprocessorName & name);

  /**
   * The the old value of an Postprocessor
   * @param name The name of the Postprocessor
   * @return The reference to the old value
   */
  PostprocessorValue & getPostprocessorValueOld(const PostprocessorName & name);

  /**
   * The the older value of an Postprocessor
   * @param name The name of the Postprocessor
   * @return The reference to the older value
   */
  PostprocessorValue & getPostprocessorValueOlder(const PostprocessorName & name);

  void storeValue(const std::string & name, PostprocessorValue value);

  /**
   * Get the map of names -> Postprocessor values. Exposed for error checking.
   */
  const std::map<std::string, PostprocessorValue *> & values() const { return _values; }

  /**
   * Copy the current Postprocessor values into old (i.e. shift it "back in time")
   */
  void copyValuesBack();

protected:
  /// Values of the Postprocessor at the current time
  std::map<std::string, PostprocessorValue *> _values;

  /// Values of the Postprocessors at the time t-1
  std::map<std::string, PostprocessorValue *> _values_old;

  /// Values of the Postprocessors at the time t-2
  std::map<std::string, PostprocessorValue *> _values_older;
};

#endif // POSTPROCESSORDATA_H
