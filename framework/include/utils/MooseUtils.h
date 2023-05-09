//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "HashMap.h"
#include "InfixIterator.h"
#include "MooseEnumItem.h"
#include "MooseError.h"
#include "MooseADWrapper.h"
#include "Moose.h"
#include "ADReal.h"
#include "ExecutablePath.h"
#include "ConsoleUtils.h"

#include "libmesh/compare_types.h"
#include "libmesh/bounding_box.h"
#include "libmesh/int_range.h"
#include "libmesh/tensor_tools.h"
#include "metaphysicl/raw_type.h"
#include "metaphysicl/metaphysicl_version.h"
#include "metaphysicl/dualnumber_decl.h"
#include "metaphysicl/dynamic_std_array_wrapper.h"
#include "timpi/standard_type.h"

// C++ includes
#include <string>
#include <vector>
#include <map>
#include <list>
#include <iterator>
#include <deque>

// Forward Declarations
class InputParameters;
class ExecFlagEnum;
class MaterialProperties;
class MaterialBase;

namespace libMesh
{
class Elem;
namespace Parallel
{
class Communicator;
}
}
class MultiMooseEnum;

namespace MooseUtils
{

std::string pathjoin(const std::string & s);

template <typename... Args>
std::string
pathjoin(const std::string & s, Args... args)
{
  if (s[s.size() - 1] == '/')
    return s + pathjoin(args...);
  return s + "/" + pathjoin(args...);
}

/// Check if the input string can be parsed into a Real
bool parsesToReal(const std::string & input);

/// Returns the location of either a local repo run_tests script - or an
/// installed test executor script if run_tests isn't found.
std::string runTestsExecutable();

/// Searches in the current working directory and then recursively up in each
/// parent directory looking for a "testroot" file.  Returns the full path to
/// the first testroot file found.
std::string findTestRoot();

/// Returns the directory of any installed inputs or the empty string if none are found.
std::string installedInputsDir(const std::string & app_name,
                               const std::string & dir_name,
                               const std::string & extra_error_msg = "");

/// Returns the directory of any installed docs/site.
std::string docsDir(const std::string & app_name);

/// Replaces all occurences of from in str with to and returns the result.
std::string replaceAll(std::string str, const std::string & from, const std::string & to);

/**
 * Replaces "LATEST" placeholders with the latest checkpoint file name.  If base_only is true, then
 * only return the base-name of the checkpoint directory - otherwise, a full mesh
 * checkpoint file path is returned.
 */
std::string convertLatestCheckpoint(std::string orig, bool base_only = true);

/// Computes and returns the Levenshtein distance between strings s1 and s2.
int levenshteinDist(const std::string & s1, const std::string & s2);

/**
 * This function will escape all of the standard C++ escape characters so that they can be printed.
 * The
 * passed in parameter is modified in place
 */
void escape(std::string & str);

/**
 * Standard scripting language trim function
 */
std::string trim(const std::string & str, const std::string & white_space = " \t\n\v\f\r");

/**
 * Python like split functions for strings.
 *
 * NOTE: This is similar to the tokenize function, but it maintains empty items, which tokenize does
 *       not. For example, "foo;bar;;" becomes {"foo", "bar", "", ""}.
 */
std::vector<std::string> split(const std::string & str,
                               const std::string & delimiter,
                               std::size_t max_count = std::numeric_limits<std::size_t>::max());
std::vector<std::string> rsplit(const std::string & str,
                                const std::string & delimiter,
                                std::size_t max_count = std::numeric_limits<std::size_t>::max());

/**
 * Python like join function for strings.
 */
template <typename T>
std::string
join(const T & strings, const std::string & delimiter)
{
  std::ostringstream oss;
  std::copy(
      strings.begin(), strings.end(), infix_ostream_iterator<std::string>(oss, delimiter.c_str()));
  return oss.str();
}

/**
 * Check the file size.
 */
std::size_t fileSize(const std::string & filename);

/**
 * This function tokenizes a path and checks to see if it contains the string to look for
 */
bool pathContains(const std::string & expression,
                  const std::string & string_to_find,
                  const std::string & delims = "/");

/**
 * Checks to see if a file is readable (exists and permissions)
 * @param filename The filename to check
 * @param check_line_endings Whether or not to see if the file contains DOS line endings.
 * @param throw_on_unreadable Whether or not to throw a MOOSE error if the file doesn't exist
 * @param check_for_git_lfs_pointer Whether or not to call a subroutine utility to make sure that
 *   the file in question is not actually a git-lfs pointer.
 * @return a Boolean indicating whether the file exists and is readable
 */
bool checkFileReadable(const std::string & filename,
                       bool check_line_endings = false,
                       bool throw_on_unreadable = true,
                       bool check_for_git_lfs_pointer = true);

/**
 * Check if the file is writable (path exists and permissions)
 * @param filename The filename you want to see if you can write to.
 * @param throw_on_unwritable Whether or not to throw a MOOSE error if the file is not writable
 * return a Boolean indicating whether the file exists and is writable
 */
bool checkFileWriteable(const std::string & filename, bool throw_on_unwritable = true);

/**
 * Check if the file is a Git-LFS pointer. When using a repository that utilizes Git-LFS,
 * it's possible that the client may not have the right packages installed in which case
 * the clone will contain plain-text files with key information for retrieving the actual
 * (large) files. This can cause odd errors since the file technically exists, is readable,
 * and even has the right name/extension. However, the content of the file will not match
 * the expected content.
 * @param file A pointer to the open filestream.
 */
bool checkForGitLFSPointer(std::ifstream & file);

/**
 * This function implements a parallel barrier function but writes progress
 * to stdout.
 */
void parallelBarrierNotify(const libMesh::Parallel::Communicator & comm, bool messaging = true);

/**
 * This function marks the begin of a section of code that is executed in serial
 * rank by rank. The section must be closed with a call to serialEnd.
 * These functions are intended for debugging use to obtain clean terminal output
 * from multiple ranks (use --keep-cout).
 * @param comm The communicator to use
 * @param warn Whether or not to warn that something is being serialized
 */
void serialBegin(const libMesh::Parallel::Communicator & comm, bool warn = true);

/**
 * Closes a section of code that is executed in serial rank by rank, and that was
 * opened with a call to serialBegin. No MPI communication can happen in this block.
 * @param comm The communicator to use
 * @param warn Whether or not to warn that something is being serialized
 */
void serialEnd(const libMesh::Parallel::Communicator & comm, bool warn = true);

/**
 * Function tests if the supplied filename as the desired extension
 * @param filename The filename to test the extension
 * @param ext The extension to test for (do not include the .)
 * @param strip_exodus_ext When true, this function ignores -s* from the end of the extension
 * @return True if the filename has the supplied extension
 */
bool hasExtension(const std::string & filename, std::string ext, bool strip_exodus_ext = false);

/**
 * Removes any file extension from the given string s (i.e. any ".[extension]" suffix of s) and
 * returns the result.
 */
std::string stripExtension(const std::string & s);

/**
 * Function for splitting path and filename
 * @param full_file A complete filename and path
 * @return A std::pair<std::string, std::string> containing the path and filename
 *
 * If the supplied filename does not contain a path, it returns "." as the path
 */
std::pair<std::string, std::string> splitFileName(std::string full_file);

/**
 * Returns the current working directory as a string. If there's a problem
 * obtaining the current working directory, this function just returns an
 * empty string. It doesn't not throw.
 */
std::string getCurrentWorkingDir();

/**
 * Recursively make directories
 * @param dir_name A complete path
 * @param throw_on_failure True to throw instead of error out when creating a directory is failed.
 *
 * The path can be relative like 'a/b/c' or absolute like '/a/b/c'.
 * The path is allowed to contain '.' or '..'.
 */
void makedirs(const std::string & dir_name, bool throw_on_failure = false);

/**
 * Recursively remove directories from inner-most when the directories are empty
 * @param dir_name A complete path
 * @param throw_on_failure True to throw instead of error out when deleting a directory is failed.
 *
 * The path can be relative like 'a/b/c' or absolute like '/a/b/c'.
 * The path is allowed to contain '.' or '..'.
 */
void removedirs(const std::string & dir_name, bool throw_on_failure = false);

/**
 * Function for converting a camel case name to a name containing underscores.
 * @param camel_case_name A string containing camel casing
 * @return a string containing no capital letters with underscores as appropriate
 */
std::string camelCaseToUnderscore(const std::string & camel_case_name);

/**
 * Function for converting an underscore name to a camel case name.
 * @param underscore_name A string containing underscores
 * @return a string containing camel casing
 */
std::string underscoreToCamelCase(const std::string & underscore_name, bool leading_upper_case);

/**
 * Function for stripping name after the file / in parser block
 */
std::string shortName(const std::string & name);

/**
 * Function for string the information before the final / in a parser block
 */
std::string baseName(const std::string & name);

/**
 * Get the hostname the current process is running on
 */
std::string hostname();

/**
 * @returns A cleaner representation of the c++ type \p cpp_type.
 */
std::string prettyCppType(const std::string & cpp_type);

/**
 * @returns A cleaner representation of the type for the given object
 */
template <typename T>
std::string
prettyCppType(const T * = nullptr)
{
  return prettyCppType(demangle(typeid(T).name()));
}

/**
 * This routine is a simple helper function for searching a map by values instead of keys
 */
template <typename T1, typename T2>
bool
doesMapContainValue(const std::map<T1, T2> & the_map, const T2 & value)
{
  for (typename std::map<T1, T2>::const_iterator iter = the_map.begin(); iter != the_map.end();
       ++iter)
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
template <typename T,
          typename T2,
          typename T3 = T,
          typename std::enable_if<ScalarTraits<T>::value && ScalarTraits<T2>::value &&
                                      ScalarTraits<T3>::value,
                                  int>::type = 0>
bool
absoluteFuzzyEqual(const T & var1,
                   const T2 & var2,
                   const T3 & tol = libMesh::TOLERANCE * libMesh::TOLERANCE)
{
  return (std::abs(MetaPhysicL::raw_value(var1) - MetaPhysicL::raw_value(var2)) <=
          MetaPhysicL::raw_value(tol));
}

/**
 * Function to check whether a variable is greater than or equal to another variable within an
 * absolute tolerance
 * @param var1 The first variable to be checked
 * @param var2 The second variable to be checked
 * @param tol The tolerance to be used
 * @return true if var1 > var2 or var1 == var2 within tol
 */
template <typename T,
          typename T2,
          typename T3 = T,
          typename std::enable_if<ScalarTraits<T>::value && ScalarTraits<T2>::value &&
                                      ScalarTraits<T3>::value,
                                  int>::type = 0>
bool
absoluteFuzzyGreaterEqual(const T & var1,
                          const T2 & var2,
                          const T3 & tol = libMesh::TOLERANCE * libMesh::TOLERANCE)
{
  return (MetaPhysicL::raw_value(var1) >=
          (MetaPhysicL::raw_value(var2) - MetaPhysicL::raw_value(tol)));
}

/**
 * Function to check whether a variable is greater than another variable within an absolute
 * tolerance
 * @param var1 The first variable to be checked
 * @param var2 The second variable to be checked
 * @param tol The tolerance to be used
 * @return true if var1 > var2 and var1 != var2 within tol
 */
template <typename T,
          typename T2,
          typename T3 = T,
          typename std::enable_if<ScalarTraits<T>::value && ScalarTraits<T2>::value &&
                                      ScalarTraits<T3>::value,
                                  int>::type = 0>
bool
absoluteFuzzyGreaterThan(const T & var1,
                         const T2 & var2,
                         const T3 & tol = libMesh::TOLERANCE * libMesh::TOLERANCE)
{
  return (MetaPhysicL::raw_value(var1) >
          (MetaPhysicL::raw_value(var2) + MetaPhysicL::raw_value(tol)));
}

/**
 * Function to check whether a variable is less than or equal to another variable within an absolute
 * tolerance
 * @param var1 The first variable to be checked
 * @param var2 The second variable to be checked
 * @param tol The tolerance to be used
 * @return true if var1 < var2 or var1 == var2 within tol
 */
template <typename T,
          typename T2,
          typename T3 = T,
          typename std::enable_if<ScalarTraits<T>::value && ScalarTraits<T2>::value &&
                                      ScalarTraits<T3>::value,
                                  int>::type = 0>
bool
absoluteFuzzyLessEqual(const T & var1,
                       const T2 & var2,
                       const T3 & tol = libMesh::TOLERANCE * libMesh::TOLERANCE)
{
  return (MetaPhysicL::raw_value(var1) <=
          (MetaPhysicL::raw_value(var2) + MetaPhysicL::raw_value(tol)));
}

/**
 * Function to check whether a variable is less than another variable within an absolute tolerance
 * @param var1 The first variable to be checked
 * @param var2 The second variable to be checked
 * @param tol The tolerance to be used
 * @return true if var1 < var2 and var1 != var2 within tol
 */
template <typename T,
          typename T2,
          typename T3 = T,
          typename std::enable_if<ScalarTraits<T>::value && ScalarTraits<T2>::value &&
                                      ScalarTraits<T3>::value,
                                  int>::type = 0>
bool
absoluteFuzzyLessThan(const T & var1,
                      const T2 & var2,
                      const T3 & tol = libMesh::TOLERANCE * libMesh::TOLERANCE)
{
  return (MetaPhysicL::raw_value(var1) <
          (MetaPhysicL::raw_value(var2) - MetaPhysicL::raw_value(tol)));
}

/**
 * Function to check whether two variables are equal within a relative tolerance
 * @param var1 The first variable to be checked
 * @param var2 The second variable to be checked
 * @param tol The relative tolerance to be used
 * @return true if var1 and var2 are equal within relative tol
 */
template <typename T, typename T2, typename T3 = Real>
bool
relativeFuzzyEqual(const T & var1,
                   const T2 & var2,
                   const T3 & tol = libMesh::TOLERANCE * libMesh::TOLERANCE)
{
  if constexpr (libMesh::ScalarTraits<T>::value ||
                libMesh::TensorTools::MathWrapperTraits<T>::value)
  {
    static_assert(libMesh::TensorTools::TensorTraits<T>::rank ==
                      libMesh::TensorTools::TensorTraits<T2>::rank,
                  "Mathematical types must be same for arguments to relativelyFuzzEqual");
    if constexpr (libMesh::TensorTools::TensorTraits<T>::rank == 0)
      return absoluteFuzzyEqual(
          var1,
          var2,
          tol * (std::abs(MetaPhysicL::raw_value(var1)) + std::abs(MetaPhysicL::raw_value(var2))));
    else if constexpr (libMesh::TensorTools::TensorTraits<T>::rank == 1)
    {
      for (const auto i : make_range(Moose::dim))
        if (!relativeFuzzyEqual(var1(i), var2(i), tol))
          return false;

      return true;
    }
    else if constexpr (libMesh::TensorTools::TensorTraits<T>::rank == 2)
    {
      for (const auto i : make_range(Moose::dim))
        for (const auto j : make_range(Moose::dim))
          if (!relativeFuzzyEqual(var1(i, j), var2(i, j), tol))
            return false;

      return true;
    }
  }
  else
  {
    // We dare to dream
    mooseAssert(var1.size() == var2.size(), "These must be the same size");
    for (const auto i : index_range(var1))
      if (!relativeFuzzyEqual(var1(i), var2(i), tol))
        return false;

    return true;
  }
}

/**
 * Function to check whether a variable is greater than or equal to another variable within a
 * relative tolerance
 * @param var1 The first variable to be checked
 * @param var2 The second variable to be checked
 * @param tol The tolerance to be used
 * @return true if var1 > var2 or var1 == var2 within relative tol
 */
template <typename T,
          typename T2,
          typename T3 = T,
          typename std::enable_if<ScalarTraits<T>::value && ScalarTraits<T2>::value &&
                                      ScalarTraits<T3>::value,
                                  int>::type = 0>
bool
relativeFuzzyGreaterEqual(const T & var1,
                          const T2 & var2,
                          const T3 & tol = libMesh::TOLERANCE * libMesh::TOLERANCE)
{
  return (absoluteFuzzyGreaterEqual(
      var1,
      var2,
      tol * (std::abs(MetaPhysicL::raw_value(var1)) + std::abs(MetaPhysicL::raw_value(var2)))));
}

/**
 * Function to check whether a variable is greater than another variable within a relative tolerance
 * @param var1 The first variable to be checked
 * @param var2 The second variable to be checked
 * @param tol The tolerance to be used
 * @return true if var1 > var2 and var1 != var2 within relative tol
 */
template <typename T,
          typename T2,
          typename T3 = T,
          typename std::enable_if<ScalarTraits<T>::value && ScalarTraits<T2>::value &&
                                      ScalarTraits<T3>::value,
                                  int>::type = 0>
bool
relativeFuzzyGreaterThan(const T & var1,
                         const T2 & var2,
                         const T3 & tol = libMesh::TOLERANCE * libMesh::TOLERANCE)
{
  return (absoluteFuzzyGreaterThan(
      var1,
      var2,
      tol * (std::abs(MetaPhysicL::raw_value(var1)) + std::abs(MetaPhysicL::raw_value(var2)))));
}

/**
 * Function to check whether a variable is less than or equal to another variable within a relative
 * tolerance
 * @param var1 The first variable to be checked
 * @param var2 The second variable to be checked
 * @param tol The tolerance to be used
 * @return true if var1 < var2 or var1 == var2 within relative tol
 */
template <typename T,
          typename T2,
          typename T3 = T,
          typename std::enable_if<ScalarTraits<T>::value && ScalarTraits<T2>::value &&
                                      ScalarTraits<T3>::value,
                                  int>::type = 0>
bool
relativeFuzzyLessEqual(const T & var1,
                       const T2 & var2,
                       const T3 & tol = libMesh::TOLERANCE * libMesh::TOLERANCE)
{
  return (absoluteFuzzyLessEqual(
      var1,
      var2,
      tol * (std::abs(MetaPhysicL::raw_value(var1)) + std::abs(MetaPhysicL::raw_value(var2)))));
}

/**
 * Function to check whether a variable is less than another variable within a relative tolerance
 * @param var1 The first variable to be checked
 * @param var2 The second variable to be checked
 * @param tol The tolerance to be used
 * @return true if var1 < var2 and var1 != var2 within relative tol
 */
template <typename T,
          typename T2,
          typename T3 = T,
          typename std::enable_if<ScalarTraits<T>::value && ScalarTraits<T2>::value &&
                                      ScalarTraits<T3>::value,
                                  int>::type = 0>
bool
relativeFuzzyLessThan(const T & var1,
                      const T2 & var2,
                      const T3 & tol = libMesh::TOLERANCE * libMesh::TOLERANCE)
{
  return (absoluteFuzzyLessThan(
      var1,
      var2,
      tol * (std::abs(MetaPhysicL::raw_value(var1)) + std::abs(MetaPhysicL::raw_value(var2)))));
}

/**
 * Taken from https://stackoverflow.com/a/257382
 * Evaluating constexpr (Has_size<T>::value) in a templated method over class T will
 * return whether T is a standard container or a singleton
 */
template <typename T>
class Has_size
{
  using Yes = char;
  struct No
  {
    char x[2];
  };

  template <typename C>
  static Yes test(decltype(&C::size));
  template <typename C>
  static No test(...);

public:
  static constexpr bool value = sizeof(test<T>(0)) == sizeof(Yes);
};

/**
 * @param value The quantity to test for zero-ness
 * @param tolerance The tolerance for testing zero-ness. The default is 1e-18 for double precision
 * configurations of libMesh/MOOSE
 * @return whether the L_infty norm of the value is (close enough to) zero
 */
template <typename T>
bool
isZero(const T & value, const Real tolerance = TOLERANCE * TOLERANCE * TOLERANCE)
{
  if constexpr (Has_size<T>::value)
  {
    for (const auto & element : value)
      if (!isZero(element, tolerance))
        return false;

    return true;
  }
  else if constexpr (libMesh::TensorTools::TensorTraits<T>::rank == 0)
    return MooseUtils::absoluteFuzzyEqual(MetaPhysicL::raw_value(value), 0, tolerance);
  else if constexpr (libMesh::TensorTools::TensorTraits<T>::rank == 1)
  {
    for (const auto i : make_range(Moose::dim))
      if (!MooseUtils::absoluteFuzzyEqual(MetaPhysicL::raw_value(value(i)), 0, tolerance))
        return false;

    return true;
  }
  else if constexpr (libMesh::TensorTools::TensorTraits<T>::rank == 2)
  {
    for (const auto i : make_range(Moose::dim))
      for (const auto j : make_range(Moose::dim))
        if (!MooseUtils::absoluteFuzzyEqual(MetaPhysicL::raw_value(value(i, j)), 0, tolerance))
          return false;

    return true;
  }
}

/**
 * Function to dump the contents of MaterialPropertyStorage for debugging purposes
 * @param props The storage item to dump, this should be
 * MaterialPropertyStorage.props()/propsOld()/propsOlder().
 *
 * Currently this only words for scalar material properties. Something to do as needed would be to
 * create a method in MaterialProperty
 * that may be overloaded to dump the type using template specialization.
 */
void MaterialPropertyStorageDump(
    const HashMap<const libMesh::Elem *, HashMap<unsigned int, MaterialProperties>> & props);

/**
 * Indents the supplied message given the prefix and color
 * @param prefix The prefix to use for indenting
 * @param message The message that will be indented
 * @param color The color to apply to the prefix (default CYAN)
 * @param indent_first_line If true this will indent the first line too (default)
 * @param post_prefix A string to append right after the prefix, defaults to a column and a space
 *
 * Takes a message like the following and indents it with another color code (see below)
 *
 * Input message:
 * COLOR_YELLOW
 * *** Warning ***
 * Something bad has happened and we want to draw attention to it with color
 * COLOR_DEFAULT
 *
 * Output message:
 * COLOR_CYAN sub_app: COLOR_YELLOW
 * COLOR_CYAN sub_app: COLOR_YELLOW *** Warning ***
 * COLOR_CYAN sub_app: COLOR_YELLOW Something bad has happened and we want to draw attention to it
 * with color
 * COLOR_DEFAULT
 *
 * Also handles single line color codes
 * COLOR_CYAN sub_app: 0 Nonlinear |R| = COLOR_GREEN 1.0e-10 COLOR_DEFAULT
 *
 * Not indenting the first line is useful in the case where the first line is actually finishing
 * the line before it.
 */
void indentMessage(const std::string & prefix,
                   std::string & message,
                   const char * color = COLOR_CYAN,
                   bool dont_indent_first_line = true,
                   const std::string & post_prefix = ": ");

/**
 * remove ANSI escape sequences for terminal color from msg
 */
std::string & removeColor(std::string & msg);

std::list<std::string> listDir(const std::string path, bool files_only = false);

bool pathExists(const std::string & path);
bool pathIsDirectory(const std::string & path);

/**
 * Retrieves the names of all of the files contained within the list of directories passed into
 * the routine. The names returned will be the paths to the files relative to the current
 * directory.
 * @param directory_list The list of directories to retrieve files from.
 */
std::list<std::string> getFilesInDirs(const std::list<std::string> & directory_list);

/**
 * Returns the most recent checkpoint or mesh file given a list of files.
 * If a suitable file isn't found the empty string is returned
 * @param checkpoint_files the list of files to analyze
 */
std::string getLatestMeshCheckpointFile(const std::list<std::string> & checkpoint_files);

std::string getLatestAppCheckpointFileBase(const std::list<std::string> & checkpoint_files);

/*
 * Checks to see if a string matches a search string
 * @param name The name to check
 * @param search_string The search string to check name against
 */
bool wildCardMatch(std::string name, std::string search_string);

/*
 * Checks to see if a candidate string matches a pattern string, permitting glob
 * wildcards (* and ?) anywhere in the pattern.
 * @param candidate The name to check
 * @param pattern The search string to check name candidate
 */
bool globCompare(const std::string & candidate,
                 const std::string & pattern,
                 std::size_t c = 0,
                 std::size_t p = 0);

template <typename T>
void
expandAllMatches(const std::vector<T> & candidates, std::vector<T> & patterns)
{
  std::set<T> expanded;
  for (const auto & p : patterns)
  {
    unsigned int found = 0;
    for (const auto & c : candidates)
      if (globCompare(c, p))
      {
        expanded.insert(c);
        found++;
      }
    if (!found)
      throw std::invalid_argument(p);
  }
  patterns.assign(expanded.begin(), expanded.end());
}

/**
 * This function will split the passed in string on a set of delimiters appending the substrings
 * to the passed in vector.  The delimiters default to "/" but may be supplied as well.  In
 * addition if min_len is supplied, the minimum token length will be greater than the supplied
 * value. T should be std::string or a MOOSE derived string class.
 */
template <typename T>
void
tokenize(const std::string & str,
         std::vector<T> & elements,
         unsigned int min_len = 1,
         const std::string & delims = "/")
{
  elements.clear();

  std::string::size_type last_pos = str.find_first_not_of(delims, 0);
  std::string::size_type pos = str.find_first_of(delims, std::min(last_pos + min_len, str.size()));

  while (last_pos != std::string::npos)
  {
    elements.push_back(str.substr(last_pos, pos - last_pos));
    // skip delims between tokens
    last_pos = str.find_first_not_of(delims, pos);
    if (last_pos == std::string::npos)
      break;
    pos = str.find_first_of(delims, std::min(last_pos + min_len, str.size()));
  }
}

/**
 *  tokenizeAndConvert splits a string using delimiter and then converts to type T.
 *  If the conversion fails tokenizeAndConvert returns false, otherwise true.
 */
template <typename T>
bool
tokenizeAndConvert(const std::string & str,
                   std::vector<T> & tokenized_vector,
                   const std::string & delimiter = " \t\n\v\f\r")
{
  std::vector<std::string> tokens;
  MooseUtils::tokenize(str, tokens, 1, delimiter);
  tokenized_vector.resize(tokens.size());
  for (unsigned int j = 0; j < tokens.size(); ++j)
  {
    std::stringstream ss(trim(tokens[j]));
    // we have to make sure that the conversion succeeded _and_ that the string
    // was fully read to avoid situations like [conversion to Real] 3.0abc to work
    if ((ss >> tokenized_vector[j]).fail() || !ss.eof())
      return false;
  }
  return true;
}

/**
 * convert takes a string representation of a number type and converts it to the number.
 * This method is here to get around deficiencies in the STL stoi and stod methods where they
 * might successfully convert part of a string to a number when we'd like to throw an error.
 */
template <typename T>
T
convert(const std::string & str, bool throw_on_failure = false)
{
  std::stringstream ss(str);
  T val;
  if ((ss >> val).fail() || !ss.eof())
  {
    std::string msg =
        std::string("Unable to convert '") + str + "' to type " + demangle(typeid(T).name());

    if (throw_on_failure)
      throw std::invalid_argument(msg);
    else
      mooseError(msg);
  }

  return val;
}

template <>
short int convert<short int>(const std::string & str, bool throw_on_failure);

template <>
unsigned short int convert<unsigned short int>(const std::string & str, bool throw_on_failure);

template <>
int convert<int>(const std::string & str, bool throw_on_failure);

template <>
unsigned int convert<unsigned int>(const std::string & str, bool throw_on_failure);

template <>
long int convert<long int>(const std::string & str, bool throw_on_failure);

template <>
unsigned long int convert<unsigned long int>(const std::string & str, bool throw_on_failure);

template <>
long long int convert<long long int>(const std::string & str, bool throw_on_failure);

template <>
unsigned long long int convert<unsigned long long int>(const std::string & str,
                                                       bool throw_on_failure);

/**
 * Create a symbolic link, if the link already exists it is replaced.
 */
void createSymlink(const std::string & target, const std::string & link);

/**
 * Remove a symbolic link, if the given filename is a link.
 */
void clearSymlink(const std::string & link);

/**
 * Convert supplied string to upper case.
 * @params name The string to convert upper case.
 */
std::string toUpper(const std::string & name);

/**
 * Convert supplied string to lower case.
 * @params name The string to convert upper case.
 */
std::string toLower(const std::string & name);

/**
 * Returns a container that contains the content of second passed in container
 * inserted into the first passed in container (set or map union).
 */
template <typename T>
T
concatenate(T c1, const T & c2)
{
  c1.insert(c2.begin(), c2.end());
  return c1;
}

/**
 * Returns a vector that contains is the concatenation of the two passed in vectors.
 */
template <typename T>
std::vector<T>
concatenate(std::vector<T> c1, const std::vector<T> & c2)
{
  c1.insert(c1.end(), c2.begin(), c2.end());
  return c1;
}

/**
 * Returns the passed in vector with the item appended to it.
 */
template <typename T>
std::vector<T>
concatenate(std::vector<T> c1, const T & item)
{
  c1.push_back(item);
  return c1;
}

/**
 * Return the number of digits for a number.
 *
 * This can foster quite a large discussion:
 * https://stackoverflow.com/questions/1489830/efficient-way-to-determine-number-of-digits-in-an-integer
 *
 * For our purposes I like the following algorithm.
 */
template <typename T>
int
numDigits(const T & num)
{
  return num > 9 ? static_cast<int>(std::log10(static_cast<double>(num))) + 1 : 1;
}

/**
 * Return the default ExecFlagEnum for MOOSE.
 */
ExecFlagEnum getDefaultExecFlagEnum();

/**
 * Robust string to integer conversion that fails for cases such at "1foo".
 * @param input The string to convert.
 * @param throw_on_failure Throw an invalid_argument exception instead of mooseError.
 */
int stringToInteger(const std::string & input, bool throw_on_failure = false);

/**
 * Linearly partition a number of items
 *
 * @param num_items The number of items to partition
 * @param num_chunks The number of chunks to partition into
 * @param chunk_id The ID of the chunk you are trying to get information about (typically the
 * current MPI rank)
 * @param num_local_items Output: The number of items for this chunk_id
 * @param local_items_begin Output: The first item for this chunk_id
 * @param local_items_end Output: One past the final item for this chunk_id
 */
void linearPartitionItems(dof_id_type num_items,
                          dof_id_type num_chunks,
                          dof_id_type chunk_id,
                          dof_id_type & num_local_items,
                          dof_id_type & local_items_begin,
                          dof_id_type & local_items_end);

/**
 * Return the chunk_id that is assigned to handle item_id
 *
 * @param num_items Global number of items to partition
 * @param num_chunks Total number of chunks to split into
 * @param item_id The item to find the chunk_id for
 * @return The chunk_id of the chunk that contains item_id
 */
processor_id_type
linearPartitionChunk(dof_id_type num_items, dof_id_type num_chunks, dof_id_type item_id);

/**
 * Wrapper around PetscGetRealPath, which is a cross-platform replacement for realpath
 */
std::string realpath(const std::string & path);

/**
 * Like python's os.path.relpath
 */
std::string relativepath(const std::string & path, const std::string & start = ".");

/**
 * Custom type trait that has a ::value of true for types that cam be use interchangably
 * with Real. Most notably it is false for complex numbers, which do not have a
 * strict ordering (and therefore no <,>,<=,>= operators).
 */
template <typename T>
struct IsLikeReal
{
  static constexpr bool value = false;
};
template <>
struct IsLikeReal<Real>
{
  static constexpr bool value = true;
};
template <>
struct IsLikeReal<DualReal>
{
  static constexpr bool value = true;
};

/**
 * Custom type trait that has a ::value of true for types that can be broadcasted
 */
template <typename T>
struct canBroadcast
{
  static constexpr bool value = std::is_base_of<TIMPI::DataType, TIMPI::StandardType<T>>::value ||
                                TIMPI::Has_buffer_type<TIMPI::Packing<T>>::value;
};

///@{ Comparison helpers that support the MooseUtils::Any wildcard which will match any value
const static struct AnyType
{
} Any;

template <typename T1, typename T2>
bool
wildcardEqual(const T1 & a, const T2 & b)
{
  return a == b;
}

template <typename T>
bool
wildcardEqual(const T &, AnyType)
{
  return true;
}
template <typename T>
bool
wildcardEqual(AnyType, const T &)
{
  return true;
}
///@}

/**
 * Find a specific pair in a container matching on first, second or both pair components
 */
template <typename C, typename M1, typename M2>
typename C::iterator
findPair(C & container, const M1 & first, const M2 & second)
{
  return std::find_if(container.begin(),
                      container.end(),
                      [&](auto & item) {
                        return wildcardEqual(first, item.first) &&
                               wildcardEqual(second, item.second);
                      });
}

/**
 * Construct a valid bounding box from 2 arbitrary points
 *
 * If you have 2 points in space and you wish to construct a bounding box, you should use
 * this method to avoid unexpected behavior of the underlying BoundingBox class in libMesh.
 * BoundingBox class expect 2 points whose coordinates are "sorted" (i.e., x-, y- and -z
 * coordinates of the first point are smaller then the corresponding coordinates of the second
 * point). If this "sorting" is not present, the BoundingBox class will build an empty box and
 * any further testing of points inside the box will fail. This method will allow you to obtain
 * the correct bounding box for any valid combination of 2 corner points of a box.
 *
 * @param p1 First corner of the constructed bounding box
 * @param p2 Second corner of the constructed bounding box
 * @return Valid bounding box
 */
BoundingBox buildBoundingBox(const Point & p1, const Point & p2);

/**
 * Utility class template for a semidynamic vector with a maximum size N
 * and a chosen dynamic size. This container avoids heap allocation and
 * is meant as a replacement for small local std::vector variables.
 * By default this class uses `value initialization`. This can be disabled
 * using the third template parameter if uninitialized storage is acceptable,
 */
template <typename T, std::size_t N, bool value_init = true>
class SemidynamicVector : public MetaPhysicL::DynamicStdArrayWrapper<T, MetaPhysicL::NWrapper<N>>
{
  typedef MetaPhysicL::DynamicStdArrayWrapper<T, MetaPhysicL::NWrapper<N>> Parent;

public:
  SemidynamicVector(std::size_t size) : Parent()
  {
    Parent::resize(size);
    if constexpr (value_init)
      for (const auto i : make_range(size))
        _data[i] = T{};
  }

  void resize(std::size_t new_size)
  {
    [[maybe_unused]] const auto old_dynamic_n = Parent::size();

    Parent::resize(new_size);

    if constexpr (value_init)
      for (const auto i : make_range(old_dynamic_n, _dynamic_n))
        _data[i] = T{};
  }

  void push_back(const T & v)
  {
    const auto old_dynamic_n = Parent::size();
    Parent::resize(old_dynamic_n + 1);
    _data[old_dynamic_n] = v;
  }

  template <typename... Args>
  void emplace_back(Args &&... args)
  {
    const auto old_dynamic_n = Parent::size();
    Parent::resize(old_dynamic_n + 1);
    (::new (&_data[old_dynamic_n]) T(std::forward<Args>(args)...));
  }

  std::size_t max_size() const { return N; }

  using Parent::_data;
  using Parent::_dynamic_n;
};

/**
 * The MooseUtils::get() specializations are used to support making
 * forwards-compatible code changes from dumb pointers to smart pointers.
 * The same line of code, e.g.
 *
 * libMesh::Parameters::Value * value = MooseUtils::get(map_iter->second);
 *
 * will then work regardless of whether map_iter->second is a dumb pointer
 * or a smart pointer. Note that the smart pointer get() functions are const
 * so they can be (ab)used to get a non-const pointer to the underlying
 * resource. We are simply following this convention here.
 */
template <typename T>
T *
get(const std::unique_ptr<T> & u)
{
  return u.get();
}

template <typename T>
T *
get(T * p)
{
  return p;
}

template <typename T>
T *
get(const std::shared_ptr<T> & s)
{
  return s.get();
}

/**
 * This method detects whether two sets intersect without building a result set.
 * It exits as soon as any intersection is detected.
 */
template <class InputIterator>
bool
setsIntersect(InputIterator first1, InputIterator last1, InputIterator first2, InputIterator last2)
{
  while (first1 != last1 && first2 != last2)
  {
    if (*first1 == *first2)
      return true;

    if (*first1 < *first2)
      ++first1;
    else if (*first1 > *first2)
      ++first2;
  }
  return false;
}

template <class T>
bool
setsIntersect(const T & s1, const T & s2)
{
  return setsIntersect(s1.begin(), s1.end(), s2.begin(), s2.end());
}

/**
 * Courtesy https://stackoverflow.com/a/8889045 and
 * https://en.cppreference.com/w/cpp/string/byte/isdigit
 * @return Whether every character in the string is a digit
 */
inline bool
isDigits(const std::string & str)
{
  return std::all_of(str.begin(), str.end(), [](unsigned char c) { return std::isdigit(c); });
}
} // MooseUtils namespace

/**
 * find, erase, length algorithm for removing a substring from a string
 */
void removeSubstring(std::string & main, const std::string & sub);

/**
 * find, erase, length algorithm for removing a substring from a copy of a string
 */
std::string removeSubstring(const std::string & main, const std::string & sub);
