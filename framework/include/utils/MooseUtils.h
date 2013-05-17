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
#ifndef MOOSEUTILS_H
#define MOOSEUTILS_H

#include <string>
#include <vector>
#include <map>

namespace MooseUtils
{
  /**
   * This function will split the passed in string on a set of delimiters appending the substrings
   * to the passed in vector.  The delimiters default to "/" but may be supplied as well.  In addition
   * if min_len is supplied, the minimum token length will be greater than the supplied value.
   */
  void tokenize(const std::string &str, std::vector<std::string> & elements, unsigned int min_len = 1, const std::string &delims = "/");

  /**
   * This function will escape all of the standard C++ escape characters so that they can be printed.  The
   * passed in parameter is modified in place
   */
  void escape(std::string &str);

  /**
   * Standard scripting language trim function
   */
  std::string trim(std::string str, const std::string &white_space = " \t\n\v\f\r");

  /**
   * This function tokenizes a path and checks to see if it contains the string to look for
   */
  bool pathContains(const std::string &expression, const std::string &string_to_find, const std::string &delims = "/");

  /**
   * Checks to see if a file is readable (exists and permissions)
   * @param filename The filename to check
   * @param check_line_endings Whether or not to see if the file contains DOS line endings.
   */
  void checkFileReadable(const std::string & filename, bool check_line_endings = false);

  /**
   * Check if the file is writable (path exists and permissions)
   * @param filename The filename you want to see if you can write to.
   */
  void checkFileWriteable(const std::string & filename);

  /**
   * This function implements a parallel barrier function but writes progress
   * to stdout.
   */
  void parallelBarrierNotify();

  /**
   * This routine is a simple helper function for searching a map by values instead of keys
   */
  template<typename T1, typename T2>
  bool doesMapContainValue(const std::map<T1, T2> & the_map, const T2 & value)
  {
    for (typename std::map<T1, T2>::const_iterator iter = the_map.begin(); iter != the_map.end(); ++iter)
      if (iter->second == value)
        return true;
    return false;
  }
}

#endif //MOOSEUTILS_H
