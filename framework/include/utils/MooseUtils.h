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
#include <sstream>
#include <algorithm>

// libMesh includes
#include "libmesh/parallel.h"

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
   * @param throw_on_unreadable Whether or not to throw a MOOSE error if the file doesn't exist
   * @return a Boolean indicating whether the file exists and is readable
   */
  bool checkFileReadable(const std::string & filename, bool check_line_endings = false, bool throw_on_unreadable = true);

  /**
   * Check if the file is writable (path exists and permissions)
   * @param filename The filename you want to see if you can write to.
   * @param throw_on_unwritable Whether or not to throw a MOOSE error if the file is not writable
   * return a Boolean indicating whether the file exists and is writable
   */
  bool checkFileWriteable(const std::string & filename, bool throw_on_unwritable = true);

  /**
   * This function implements a parallel barrier function but writes progress
   * to stdout.
   */
  void parallelBarrierNotify(const libMesh::Parallel::Communicator & comm);

  /**
   * Function tests if the supplied filename as the desired extension
   * @param filename The filename to test the extension
   * @param ext The extension to test for (do not include the .)
   * @param strip_exodus_ext When true, this function ignores -s* from the end of the extension
   * @return True if the filename has the supplied extension
   */
  bool hasExtension(const std::string & filename, std::string ext, bool strip_exodus_ext = false);

  /**
   * Function for splitting path and filename
   * @param full_name A complete filename and path
   * @param A std::pair<std::string, std::string> containing the path and filename
   *
   * If the supplied filename does not contain a path, it returns "." as the path
   */
  std::pair<std::string, std::string> splitFileName(std::string full_file);

  /**
   * Function for converting a camel case name to a name containing underscores.
   * @param camel_case_name A string containing camel casing
   * @return a string containing no capital letters with underscores as appropriate
   */
  std::string
  camelCaseToUnderscore(const std::string & camel_case_name);

  /**
   * Function for converting an underscore name to a camel case name.
   * @param underscore_name A string containing underscores
   * @return a string containing camel casing
   */
  std::string
  underscoreToCamelCase(const std::string & underscore_name, bool leading_upper_case);

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

  /**
   * Function to check whether two variables are equal within an absolute tolerance
   * @param var1 The first variable to be checked
   * @param var2 The second variable to be checked
   * @param tol The tolerance to be used
   * @return true if var1 and var2 are equal within tol
   */
  bool absoluteFuzzyEqual(const libMesh::Real & var1, const libMesh::Real & var2, const libMesh::Real & tol = libMesh::TOLERANCE*libMesh::TOLERANCE);

  /**
   * Function to check whether a variable is greater than or equal to another variable within an absolute tolerance
   * @param var1 The first variable to be checked
   * @param var2 The second variable to be checked
   * @param tol The tolerance to be used
   * @return true if var1 > var2 or var1 == var2 within tol
   */
  bool absoluteFuzzyGreaterEqual(const libMesh::Real & var1, const libMesh::Real & var2, const libMesh::Real & tol = libMesh::TOLERANCE*libMesh::TOLERANCE);

  /**
   * Function to check whether a variable is greater than another variable within an absolute tolerance
   * @param var1 The first variable to be checked
   * @param var2 The second variable to be checked
   * @param tol The tolerance to be used
   * @return true if var1 > var2 and var1 != var2 within tol
   */
  bool absoluteFuzzyGreaterThan(const libMesh::Real & var1, const libMesh::Real & var2, const libMesh::Real & tol = libMesh::TOLERANCE*libMesh::TOLERANCE);

  /**
   * Function to check whether a variable is less than or equal to another variable within an absolute tolerance
   * @param var1 The first variable to be checked
   * @param var2 The second variable to be checked
   * @param tol The tolerance to be used
   * @return true if var1 < var2 or var1 == var2 within tol
   */
  bool absoluteFuzzyLessEqual(const libMesh::Real & var1, const libMesh::Real & var2, const libMesh::Real & tol = libMesh::TOLERANCE*libMesh::TOLERANCE);

  /**
   * Function to check whether a variable is less than another variable within an absolute tolerance
   * @param var1 The first variable to be checked
   * @param var2 The second variable to be checked
   * @param tol The tolerance to be used
   * @return true if var1 < var2 and var1 != var2 within tol
   */
  bool absoluteFuzzyLessThan(const libMesh::Real & var1, const libMesh::Real & var2, const libMesh::Real & tol = libMesh::TOLERANCE*libMesh::TOLERANCE);

  /**
   * Function to check whether two variables are equal within a relative tolerance
   * @param var1 The first variable to be checked
   * @param var2 The second variable to be checked
   * @param tol The relative tolerance to be used
   * @return true if var1 and var2 are equal within relative tol
   */
  bool relativeFuzzyEqual(const libMesh::Real & var1, const libMesh::Real & var2, const libMesh::Real & tol = libMesh::TOLERANCE*libMesh::TOLERANCE);

  /**
   * Function to check whether a variable is greater than or equal to another variable within a relative tolerance
   * @param var1 The first variable to be checked
   * @param var2 The second variable to be checked
   * @param tol The tolerance to be used
   * @return true if var1 > var2 or var1 == var2 within relative tol
   */
  bool relativeFuzzyGreaterEqual(const libMesh::Real & var1, const libMesh::Real & var2, const libMesh::Real & tol = libMesh::TOLERANCE*libMesh::TOLERANCE);

  /**
   * Function to check whether a variable is greater than another variable within a relative tolerance
   * @param var1 The first variable to be checked
   * @param var2 The second variable to be checked
   * @param tol The tolerance to be used
   * @return true if var1 > var2 and var1 != var2 within relative tol
   */
  bool relativeFuzzyGreaterThan(const libMesh::Real & var1, const libMesh::Real & var2, const libMesh::Real & tol = libMesh::TOLERANCE*libMesh::TOLERANCE);

  /**
   * Function to check whether a variable is less than or equal to another variable within a relative tolerance
   * @param var1 The first variable to be checked
   * @param var2 The second variable to be checked
   * @param tol The tolerance to be used
   * @return true if var1 < var2 or var1 == var2 within relative tol
   */
  bool relativeFuzzyLessEqual(const libMesh::Real & var1, const libMesh::Real & var2, const libMesh::Real & tol = libMesh::TOLERANCE*libMesh::TOLERANCE);

  /**
   * Function to check whether a variable is less than another variable within a relative tolerance
   * @param var1 The first variable to be checked
   * @param var2 The second variable to be checked
   * @param tol The tolerance to be used
   * @return true if var1 < var2 and var1 != var2 within relative tol
   */
  bool relativeFuzzyLessThan(const libMesh::Real & var1, const libMesh::Real & var2, const libMesh::Real & tol = libMesh::TOLERANCE*libMesh::TOLERANCE);

}

#endif //MOOSEUTILS_H
