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

#ifndef YAMLFORMATTER_H
#define YAMLFORMATTER_H

#include "SyntaxTree.h"
#include <sstream>
#include <iterator>

/**
 * This class produces produces a yaml dump of the InputFileParameters that is machine parsable by any YAML formatter.
 */
class YAMLFormatter : public SyntaxTree
{
public:
  YAMLFormatter(bool dump_mode);

  virtual std::string preamble() const;
  virtual std::string postscript() const;

  virtual std::string preTraverse(short depth) const;
  virtual std::string printBlockOpen(const std::string &name, short depth, const std::string & doc, const std::string & base) const;
  virtual std::string printBlockClose(const std::string &name, short depth) const;
  virtual std::string printParams(const std::string &prefix, const std::string &fully_qualified_name, InputParameters &params, short depth, const std::string &search_string, bool &found);


  void setMooseBase(const std::string & moose_base) { _moose_base = moose_base; }

protected:
  bool _dump_mode;

  std::string _moose_base;


  /**
   * Method for building an output string that accounts for specific types (e.g., Point)
   * @param output Reference to the output string
   * @param iter InputParameters iterator that is being output
   */
  void buildOutputString(std::ostringstream & output, const std::iterator_traits<InputParameters::iterator>::value_type & p);
};

#endif /* YAMLFORMATTER_H */
