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

#include "XTermConstants.h"

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
  void parallelBarrierNotify(const libMesh::Parallel::Communicator & comm);

  /**
   * Function tests if the supplied filename as the desired extension
   * @param filename The filename to test the extension
   * @param ext The extension to test for (do not include the .)
   * @return True if the filename has the supplied extension
   */
  bool hasExtension(const std::string & filename, std::string ext);

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
   * Returns a character string to produce a specific color in terminals supporting
   * color. The list of color constants is available in XTermConstants.h
   * @param color (from XTermConstants.h)
   * @param text The output to be converted to text and colored
   * @param use_color A convenience flag using or not using color (see src/outputs/Console.C)
   */
  template <typename T>
  std::string
  colorText(std::string color, T text, bool use_color = true)
  {
    // Define a stream to output
    std::ostringstream oss;
    oss << std::scientific;

    // Output the value to the stream
    if (use_color)
      oss << color << text << COLOR_DEFAULT;
    else
      oss << text;

    // Return string
    return oss.str();
  }






  /*
   * a function object that allows to compare
   * the iterators by the value they point to
   *
   * Inspired by behzad.nouri from http://stackoverflow.com/questions/1577475/c-sorting-and-keeping-track-of-indexes
   */
  template <class RAIter, class Compare>
  class IterSortComp
  {
  public:
    IterSortComp(Compare comp) : m_comp(comp) {}

    inline bool operator() (const RAIter & i, const RAIter & j) const
    {
      return m_comp(*i, *j);
    }

  private:
    const Compare m_comp;
  };

  template <class INIter, class RAIter, class Compare>
  void itersort(INIter first, INIter last, std::vector<RAIter> & idx, Compare comp)
  {
    idx.resize(std::distance(first, last));
    for (typename std::vector<RAIter>::iterator j = idx.begin(); first != last; ++j, ++first)
      *j = first;

    std::sort(idx.begin(), idx.end(), IterSortComp<RAIter, Compare>(comp));
  }

  template <class T, class U, class Compare>
  void indirectSortInto(const T & sort_this, const U &  by_this, T & into_this, Compare comp)
  {
    std::vector<typename U::const_iterator > idx;
    itersort(by_this.begin(), by_this.end(), idx, comp);

    into_this.resize(sort_this.size());

    for (unsigned int i=0; i<idx.size(); i++)
      into_this[i] = sort_this[std::distance(by_this.begin(), idx[i])];

  }

  template <class T, class Compare>
  void getSortIndices(const T & sort_by, Compare comp, std::vector<unsigned int> & sorted_indices)
  {
    std::vector<typename T::const_iterator > idx;
    itersort(sort_by.begin(), sort_by.end(), idx, comp);

    sorted_indices.resize(idx.size());

    for (unsigned int i=0; i<idx.size(); i++)
      sorted_indices[i] = std::distance(sort_by.begin(), idx[i]);
  }
}

#endif //MOOSEUTILS_H
