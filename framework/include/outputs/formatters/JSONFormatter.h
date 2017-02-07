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

#ifndef JSONFORMATTER_H
#define JSONFORMATTER_H

#include "SyntaxTree.h"
#include <sstream>
#include <iterator>
#include "json/json.h"

/**
 * This class produces produces a JSON dump of the InputFileParameters that is machine parsable by any JSON formatter.
 */
class JSONFormatter : public SyntaxTree
{
public:
  JSONFormatter(bool dump_mode);

  virtual std::string postscript() const override;

  virtual std::string printBlockOpen(const std::string &name, short depth, const std::string & doc) override;
  virtual std::string printBlockClose(const std::string &name, short depth) const override;
  virtual std::string printParams(const std::string &prefix, const std::string &fully_qualified_name, InputParameters &params, short depth, const std::string &search_string, bool &found) override;

protected:
  /**
   * Splits a path like "foo/bar/other" into a vector with entries "foo", "bar", "other"
   * @param long_name Path to break up
   */
  std::vector<std::string> splitPath(const std::string& long_name) const;
  /**
   * Gets the JSON entry for the specified path and creates it if it doesn't exist
   * @param full_path Path to the entry
   */
  Json::Value _json;
  Json::Value& getJson(const std::string& full_path);
  bool _dump_mode;

  /**
   * Method for building an output string that accounts for specific types (e.g., Point)
   * @param output Reference to the output string
   * @param iter InputParameters iterator that is being output
   */
  void buildOutputString(std::ostringstream & output, const std::iterator_traits<InputParameters::iterator>::value_type & p);
};

#endif /* JSONFORMATTER_H */
