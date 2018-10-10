//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MOOSEUTILS_H
#define MOOSEUTILS_H

// MOOSE includes
#include "HashMap.h"
#include "MaterialProperty.h" // MaterialProperties
#include "InfixIterator.h"
#include "MooseEnumItem.h"

// C++ includes
#include <string>
#include <vector>
#include <map>
#include <list>
#include <iterator>

// Forward Declarations
class InputParameters;
class ExecFlagEnum;

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
 * @return a Boolean indicating whether the file exists and is readable
 */
bool checkFileReadable(const std::string & filename,
                       bool check_line_endings = false,
                       bool throw_on_unreadable = true);

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
 * Removes any file extension from the fiven string s (i.e. any ".[extension]" suffix of s) and
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
bool absoluteFuzzyEqual(const libMesh::Real & var1,
                        const libMesh::Real & var2,
                        const libMesh::Real & tol = libMesh::TOLERANCE * libMesh::TOLERANCE);

/**
 * Function to check whether a variable is greater than or equal to another variable within an
 * absolute tolerance
 * @param var1 The first variable to be checked
 * @param var2 The second variable to be checked
 * @param tol The tolerance to be used
 * @return true if var1 > var2 or var1 == var2 within tol
 */
bool absoluteFuzzyGreaterEqual(const libMesh::Real & var1,
                               const libMesh::Real & var2,
                               const libMesh::Real & tol = libMesh::TOLERANCE * libMesh::TOLERANCE);

/**
 * Function to check whether a variable is greater than another variable within an absolute
 * tolerance
 * @param var1 The first variable to be checked
 * @param var2 The second variable to be checked
 * @param tol The tolerance to be used
 * @return true if var1 > var2 and var1 != var2 within tol
 */
bool absoluteFuzzyGreaterThan(const libMesh::Real & var1,
                              const libMesh::Real & var2,
                              const libMesh::Real & tol = libMesh::TOLERANCE * libMesh::TOLERANCE);

/**
 * Function to check whether a variable is less than or equal to another variable within an absolute
 * tolerance
 * @param var1 The first variable to be checked
 * @param var2 The second variable to be checked
 * @param tol The tolerance to be used
 * @return true if var1 < var2 or var1 == var2 within tol
 */
bool absoluteFuzzyLessEqual(const libMesh::Real & var1,
                            const libMesh::Real & var2,
                            const libMesh::Real & tol = libMesh::TOLERANCE * libMesh::TOLERANCE);

/**
 * Function to check whether a variable is less than another variable within an absolute tolerance
 * @param var1 The first variable to be checked
 * @param var2 The second variable to be checked
 * @param tol The tolerance to be used
 * @return true if var1 < var2 and var1 != var2 within tol
 */
bool absoluteFuzzyLessThan(const libMesh::Real & var1,
                           const libMesh::Real & var2,
                           const libMesh::Real & tol = libMesh::TOLERANCE * libMesh::TOLERANCE);

/**
 * Function to check whether two variables are equal within a relative tolerance
 * @param var1 The first variable to be checked
 * @param var2 The second variable to be checked
 * @param tol The relative tolerance to be used
 * @return true if var1 and var2 are equal within relative tol
 */
bool relativeFuzzyEqual(const libMesh::Real & var1,
                        const libMesh::Real & var2,
                        const libMesh::Real & tol = libMesh::TOLERANCE * libMesh::TOLERANCE);

/**
 * Function to check whether a variable is greater than or equal to another variable within a
 * relative tolerance
 * @param var1 The first variable to be checked
 * @param var2 The second variable to be checked
 * @param tol The tolerance to be used
 * @return true if var1 > var2 or var1 == var2 within relative tol
 */
bool relativeFuzzyGreaterEqual(const libMesh::Real & var1,
                               const libMesh::Real & var2,
                               const libMesh::Real & tol = libMesh::TOLERANCE * libMesh::TOLERANCE);

/**
 * Function to check whether a variable is greater than another variable within a relative tolerance
 * @param var1 The first variable to be checked
 * @param var2 The second variable to be checked
 * @param tol The tolerance to be used
 * @return true if var1 > var2 and var1 != var2 within relative tol
 */
bool relativeFuzzyGreaterThan(const libMesh::Real & var1,
                              const libMesh::Real & var2,
                              const libMesh::Real & tol = libMesh::TOLERANCE * libMesh::TOLERANCE);

/**
 * Function to check whether a variable is less than or equal to another variable within a relative
 * tolerance
 * @param var1 The first variable to be checked
 * @param var2 The second variable to be checked
 * @param tol The tolerance to be used
 * @return true if var1 < var2 or var1 == var2 within relative tol
 */
bool relativeFuzzyLessEqual(const libMesh::Real & var1,
                            const libMesh::Real & var2,
                            const libMesh::Real & tol = libMesh::TOLERANCE * libMesh::TOLERANCE);

/**
 * Function to check whether a variable is less than another variable within a relative tolerance
 * @param var1 The first variable to be checked
 * @param var2 The second variable to be checked
 * @param tol The tolerance to be used
 * @return true if var1 < var2 and var1 != var2 within relative tol
 */
bool relativeFuzzyLessThan(const libMesh::Real & var1,
                           const libMesh::Real & var2,
                           const libMesh::Real & tol = libMesh::TOLERANCE * libMesh::TOLERANCE);

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
 *
 * Takes a message like the following and indents it with another color code (see below)
 *
 * Input messsage:
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
 * COLOR_CYAN sub_app: 0 Nonline |R| = COLOR_GREEN 1.0e-10 COLOR_DEFAULT
 */
void
indentMessage(const std::string & prefix, std::string & message, const char * color = COLOR_CYAN);

/**
 * remove ANSI escape sequences for teminal color from msg
 */
std::string & removeColor(std::string & msg);

std::list<std::string> listDir(const std::string path, bool files_only = false);

bool pathExists(const std::string & path);

/**
 * Retrieves the names of all of the files contained within the list of directories passed into the
 * routine.
 * The names returned will be the paths to the files relative to the current directory.
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

/**
 * This function will split the passed in string on a set of delimiters appending the substrings
 * to the passed in vector.  The delimiters default to "/" but may be supplied as well.  In addition
 * if min_len is supplied, the minimum token length will be greater than the supplied value.
 * T should be std::string or a MOOSE derived string class.
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
} // MooseUtils namespace

#endif // MOOSEUTILS_H
