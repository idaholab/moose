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

// MOOSE includes
#include "HashMap.h"
#include "MaterialProperty.h" // MaterialProperties

// C++ includes
#include <string>
#include <vector>
#include <map>
#include <list>

// Forward Declarations
class InputParameters;
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
/**
 * This function will escape all of the standard C++ escape characters so that they can be printed.
 * The
 * passed in parameter is modified in place
 */
void escape(std::string & str);

/**
 * Standard scripting language trim function
 */
std::string trim(std::string str, const std::string & white_space = " \t\n\v\f\r");

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
void parallelBarrierNotify(const libMesh::Parallel::Communicator & comm);

/**
 * This function marks the begin of a section of code that is executed in serial
 * rank by rank. The section must be closed with a call to serialEnd.
 * These functions are intended for debugging use to obtain clean terminal output
 * from multiple ranks (use --keep-cout).
 */
void serialBegin(const libMesh::Parallel::Communicator & comm);

/**
 * Closes a section of code that is executed in serial rank by rank, and that was
 * opened with a call to serialBegin. No MPI communication can happen in this block.
 */
void serialEnd(const libMesh::Parallel::Communicator & comm);

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

/**
 * Retrieves the names of all of the files contained within the list of directories passed into the
 * routine.
 * The names returned will be the paths to the files relative to the current directory.
 * @param directory_list The list of directories to retrieve files from.
 */
std::list<std::string> getFilesInDirs(const std::list<std::string> & directory_list);

/**
 * Returns the most recent checkpoint file given a list of files.
 * If a suitable file isn't found the empty string is returned
 * @param checkpoint_files the list of files to analyze
 */
std::string getRecoveryFileBase(const std::list<std::string> & checkpoint_files);

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
 * T should be std::string or a MOOSE derivied string class.
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
    std::stringstream ss(tokens[j]);
    // we have to make sure that the conversion succeeded _and_ that the string
    // was fully read to avoid situations like [conversion to Real] 3.0abc to work
    if ((ss >> tokenized_vector[j]).fail() || !ss.eof())
      return false;
  }
  return true;
}

/**
 * Returns the default execute_on MultiMooseEnum.
 * @param default_names Space separated list to set the default execute flags.
 */
MultiMooseEnum createExecuteOnEnum(const std::set<ExecFlagType> & set_flags = {},
                                   const std::set<ExecFlagType> & add_flags = {},
                                   const std::set<ExecFlagType> & remove_flags = {});

///@
/**
 * Add/removes/sets the given execute flags to the InputParameters or MultiMooseEnum object.
 * @param params/exec_enum The InputParameters or MultiMooseEnum object to modify.
 * @param flags A set of ExecFlagType to append as possible values.
 *
 * The add/remove methods also automatically update the doc string for the "execute_on" parameter.
 */
void addExecuteOnFlags(InputParameters & params, const std::set<ExecFlagType> & flags);
void addExecuteOnFlags(MultiMooseEnum & exec_enum, const std::set<ExecFlagType> & flags);
void removeExecuteOnFlags(InputParameters & params, const std::set<ExecFlagType> & flags);
void removeExecuteOnFlags(MultiMooseEnum & exec_enum, const std::set<ExecFlagType> & flags);
void setExecuteOnFlags(InputParameters & params, const std::set<ExecFlagType> & flags);
void setExecuteOnFlags(MultiMooseEnum & exec_enum, const std::set<ExecFlagType> & flags);
///@}

/**
 */
std::map<ExecFlagType, std::string>::const_iterator getExecuteOnFlag(const ExecFlagType & flag);

/**
 */
MultiMooseEnum & getExecuteOnEnum(InputParameters & parameters);

/**
 * Return a documentation string with the available and default execute_on options.
 */
std::string getExecuteOnEnumDocString(const MultiMooseEnum & exec_enum);
}

#endif // MOOSEUTILS_H
