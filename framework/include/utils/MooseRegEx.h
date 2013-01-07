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

#ifndef MOOSEREGEX_H
#define MOOSEREGEX_H

#include "trex.h"

#include <string>
#include <vector>

/**
 * This class wraps the T-Rex "tiny regular expression" library distributed with MOOSE.
 * It supports the common POSIX character classes (with Perl syntax), greedy quantifiers,
 * basic anchors (^, $), and capturing.  Non-capturing parenthesis are also supported.
 */
class MooseRegEx
{
public:
  // Default constructor, builds an uninitialized instance of the class (no RE pattern)
  MooseRegEx();

  /**
   * This version of the constructor takes a pattern to compile.
   * @param pattern  An RE pattern to compile.  Note: You will need to escape your backslashes (.i.e. "\\w+")
   */
  MooseRegEx(const std::string & pattern);

  ~MooseRegEx();

  /**
   * This method will populate the instance with a new RE pattern to search with.  If the instance
   * already has an active pattern, it will be cleaned up and replaced with the new pattern.
   * @param pattern  An RE pattern to compile.
   */
  void compile(const std::string & pattern);

  /**
   * This method applies the RegEx to the passed in string.
   * @param str to apply the pattern against for searching
   * @return Boolean indicating whether or not the pattern was found
   */
  bool search(const std::string & str) const;

  /**
   * This method applies the RegEx to the passed in string, filling in the passed in vector
   * with the whole match and captured sub-expressions as appropriate.
   * @param str to apply the pattern against for searching
   * @param groups A vector that will be overwritten with results
   *        [0] - will contain the full match if successful
   *        [1-n] - will contain the captured sub-expressions.
   * @return Boolean indicating whether or not the pattern was found
   */
  bool search(const std::string & str, std::vector<std::string> & groups) const;

  /**
   * This method will find all matching patterns in the passed in string populating "groups".
   * Currently capturing parenthesis do not yield additional information when using this routine.
   * @param str to apply the pattern against for searching
   * @param groups A vector that will be overwritten with results
   *       [0-n] each entry will contain a match if any are found.
   * @return Boolean indicating whether or not the pattern was found
   */
  bool findall(const std::string & str, std::vector<std::string> & groups) const;

protected:

  /// This method clears memory used by the pattern, called from the destructor and compile
  void cleanUp();

  /// The dynamic RegEx object
  TRex *_re;
};

#endif // MOOSEREGEX_H
