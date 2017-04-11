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

// MOOSE includes
#include "MooseUtils.h"
#include "MooseError.h"
#include "MaterialProperty.h"
#include "MultiMooseEnum.h"
#include "InputParameters.h"

// libMesh includes
#include "libmesh/elem.h"

// External includes
#include "pcrecpp.h"
#include "tinydir.h"

// C++ includes
#include <iostream>
#include <fstream>
#include <istream>
#include <iterator>

// System includes
#include <sys/stat.h>

namespace MooseUtils
{

void
escape(std::string & str)
{
  std::map<char, std::string> escapes;
  escapes['\a'] = "\\a";
  escapes['\b'] = "\\b";
  escapes['\f'] = "\\f";
  escapes['\n'] = "\\n";
  escapes['\t'] = "\\t";
  escapes['\v'] = "\\v";
  escapes['\r'] = "\\r";

  for (const auto & it : escapes)
    for (size_t pos = 0; (pos = str.find(it.first, pos)) != std::string::npos;
         pos += it.second.size())
      str.replace(pos, 1, it.second);
}

std::string
trim(std::string str, const std::string & white_space)
{
  std::string r = str.erase(str.find_last_not_of(white_space) + 1);
  return r.erase(0, r.find_first_not_of(white_space));
}

bool
pathContains(const std::string & expression,
             const std::string & string_to_find,
             const std::string & delims)
{
  std::vector<std::string> elements;
  tokenize(expression, elements, 0, delims);

  std::vector<std::string>::iterator found_it =
      std::find(elements.begin(), elements.end(), string_to_find);
  if (found_it != elements.end())
    return true;
  else
    return false;
}

bool
checkFileReadable(const std::string & filename, bool check_line_endings, bool throw_on_unreadable)
{
  std::ifstream in(filename.c_str(), std::ifstream::in);
  if (in.fail())
  {
    if (throw_on_unreadable)
      mooseError(
          (std::string("Unable to open file \"") + filename +
           std::string("\". Check to make sure that it exists and that you have read permission."))
              .c_str());
    else
      return false;
  }

  if (check_line_endings)
  {
    std::istream_iterator<char> iter(in);
    std::istream_iterator<char> eos;
    in >> std::noskipws;
    while (iter != eos)
      if (*iter++ == '\r')
        mooseError(filename + " contains Windows(DOS) line endings which are not supported.");
  }

  in.close();

  return true;
}

bool
checkFileWriteable(const std::string & filename, bool throw_on_unwritable)
{
  std::ofstream out(filename.c_str(), std::ofstream::out);
  if (out.fail())
  {
    if (throw_on_unwritable)
      mooseError(
          (std::string("Unable to open file \"") + filename +
           std::string("\". Check to make sure that it exists and that you have write permission."))
              .c_str());
    else
      return false;
  }

  out.close();

  return true;
}

void
parallelBarrierNotify(const Parallel::Communicator & comm)
{
  processor_id_type slave_processor_id;

  if (comm.rank() == 0)
  {
    // The master process is already through, so report it
    Moose::out << "Jobs complete: 1/" << comm.size() << (1 == comm.size() ? "\n" : "\r")
               << std::flush;
    for (unsigned int i = 2; i <= comm.size(); ++i)
    {
      comm.receive(MPI_ANY_SOURCE, slave_processor_id);
      Moose::out << "Jobs complete: " << i << "/" << comm.size() << (i == comm.size() ? "\n" : "\r")
                 << std::flush;
    }
  }
  else
  {
    slave_processor_id = comm.rank();
    comm.send(0, slave_processor_id);
  }

  comm.barrier();
}

void
serialBegin(const libMesh::Parallel::Communicator & comm)
{
  // unless we are the first processor...
  if (comm.rank() > 0)
  {
    // ...wait for the previous processor to finish
    int dummy = 0;
    comm.receive(comm.rank() - 1, dummy);
  }
  else
    mooseWarning("Entering serial execution block (use only for debugging)");
}

void
serialEnd(const libMesh::Parallel::Communicator & comm)
{
  // unless we are the last processor...
  if (comm.rank() + 1 < comm.size())
  {
    // ...notify the next processor of its turn
    int dummy = 0;
    comm.send(comm.rank() + 1, dummy);
  }
  else
    mooseWarning("Leaving serial execution block (use only for debugging)");
}

bool
hasExtension(const std::string & filename, std::string ext, bool strip_exodus_ext)
{
  // Extract the extension, w/o the '.'
  std::string file_ext;
  if (strip_exodus_ext)
  {
    pcrecpp::RE re(
        ".*\\.([^\\.]*?)(?:-s\\d+)?\\s*$"); // capture the complete extension, ignoring -s*
    re.FullMatch(filename, &file_ext);
  }
  else
  {
    pcrecpp::RE re(".*\\.([^\\.]*?)\\s*$"); // capture the complete extension
    re.FullMatch(filename, &file_ext);
  }

  // Perform the comparision
  if (file_ext == ext)
    return true;
  else
    return false;
}

std::pair<std::string, std::string>
splitFileName(std::string full_file)
{
  // Error if path ends with /
  if (full_file.empty() || *full_file.rbegin() == '/')
    mooseError("Invalid full file name: ", full_file);

  // Define the variables to output
  std::string path;
  std::string file;

  // Locate the / sepearting the file from path
  std::size_t found = full_file.find_last_of("/");

  // If no / is found used "." for the path, otherwise seperate the two
  if (found == std::string::npos)
  {
    path = ".";
    file = full_file;
  }
  else
  {
    path = full_file.substr(0, found);
    file = full_file.substr(found + 1);
  }

  // Return the path and file as a pair
  return std::pair<std::string, std::string>(path, file);
}

std::string
camelCaseToUnderscore(const std::string & camel_case_name)
{
  string replaced = camel_case_name;
  // Put underscores in front of each contiguous set of capital letters
  pcrecpp::RE("(?!^)([A-Z]+)").GlobalReplace("_\\1", &replaced);

  // Convert all capital letters to lower case
  std::transform(replaced.begin(), replaced.end(), replaced.begin(), ::tolower);
  return replaced;
}

std::string
underscoreToCamelCase(const std::string & underscore_name, bool leading_upper_case)
{
  pcrecpp::StringPiece input(underscore_name);
  pcrecpp::RE re("([^_]*)(_|$)");

  std::string result;
  std::string us, not_us;
  bool make_upper = leading_upper_case;
  while (re.Consume(&input, &not_us, &us))
  {
    if (not_us.length() > 0)
    {
      if (make_upper)
      {
        result += std::toupper(not_us[0]);
        if (not_us.length() > 1)
          result += not_us.substr(1);
      }
      else
        result += not_us;
    }
    if (us == "")
      break;

    // Toggle flag so next match is upper cased
    make_upper = true;
  }

  return result;
}

std::string
shortName(const std::string & name)
{
  return name.substr(name.find_last_of('/') != std::string::npos ? name.find_last_of('/') + 1 : 0);
}

std::string
baseName(const std::string & name)
{
  return name.substr(0, name.find_last_of('/') != std::string::npos ? name.find_last_of('/') : 0);
}

bool
absoluteFuzzyEqual(const Real & var1, const Real & var2, const Real & tol)
{
  return (std::abs(var1 - var2) <= tol);
}

bool
absoluteFuzzyGreaterEqual(const Real & var1, const Real & var2, const Real & tol)
{
  return (var1 >= (var2 - tol));
}

bool
absoluteFuzzyGreaterThan(const Real & var1, const Real & var2, const Real & tol)
{
  return (var1 > (var2 + tol));
}

bool
absoluteFuzzyLessEqual(const Real & var1, const Real & var2, const Real & tol)
{
  return (var1 <= (var2 + tol));
}

bool
absoluteFuzzyLessThan(const Real & var1, const Real & var2, const Real & tol)
{
  return (var1 < (var2 - tol));
}

bool
relativeFuzzyEqual(const Real & var1, const Real & var2, const Real & tol)
{
  return (absoluteFuzzyEqual(var1, var2, tol * (std::abs(var1) + std::abs(var2))));
}

bool
relativeFuzzyGreaterEqual(const Real & var1, const Real & var2, const Real & tol)
{
  return (absoluteFuzzyGreaterEqual(var1, var2, tol * (std::abs(var1) + std::abs(var2))));
}

bool
relativeFuzzyGreaterThan(const Real & var1, const Real & var2, const Real & tol)
{
  return (absoluteFuzzyGreaterThan(var1, var2, tol * (std::abs(var1) + std::abs(var2))));
}

bool
relativeFuzzyLessEqual(const Real & var1, const Real & var2, const Real & tol)
{
  return (absoluteFuzzyLessEqual(var1, var2, tol * (std::abs(var1) + std::abs(var2))));
}

bool
relativeFuzzyLessThan(const Real & var1, const Real & var2, const Real & tol)
{
  return (absoluteFuzzyLessThan(var1, var2, tol * (std::abs(var1) + std::abs(var2))));
}

void
MaterialPropertyStorageDump(
    const HashMap<const libMesh::Elem *, HashMap<unsigned int, MaterialProperties>> & props)
{
  // Loop through the elements
  for (const auto & elem_it : props)
  {
    Moose::out << "Element " << elem_it.first->id() << '\n';

    // Loop through the sides
    for (const auto & side_it : elem_it.second)
    {
      Moose::out << "  Side " << side_it.first << '\n';

      // Loop over properties
      unsigned int cnt = 0;
      for (const auto & mat_prop : side_it.second)
      {
        MaterialProperty<Real> * mp = dynamic_cast<MaterialProperty<Real> *>(mat_prop);
        if (mp)
        {
          Moose::out << "    Property " << cnt << '\n';
          cnt++;

          // Loop over quadrature points
          for (unsigned int qp = 0; qp < mp->size(); ++qp)
            Moose::out << "      prop[" << qp << "] = " << (*mp)[qp] << '\n';
        }
      }
    }
  }
}

std::string &
removeColor(std::string & msg)
{
  pcrecpp::RE re("(\\33\\[3[0-7]m))", pcrecpp::DOTALL());
  re.GlobalReplace(std::string(""), &msg);
  return msg;
}

void
indentMessage(const std::string & prefix,
              std::string & message,
              const char * color /*= COLOR_CYAN*/)
{
  // First we need to see if the message we need to indent (with color) also contains color codes
  // that span lines.
  // The code matches all of the XTERM constants (see XTermConstants.h). If it does, then we'll work
  // on formatting
  // each colored multiline chunk one at a time with the right codes.
  std::string colored_message;
  std::string curr_color = COLOR_DEFAULT; // tracks last color code before newline
  std::string line, color_code;

  std::istringstream iss(message);
  for (std::string line; std::getline(iss, line);) // loop over each line
  {
    const static pcrecpp::RE match_color(".*(\\33\\[3\\dm)((?!\\33\\[3\\d)[^\n])*");
    pcrecpp::StringPiece line_piece(line);
    match_color.FindAndConsume(&line_piece, &color_code);
    colored_message += color + prefix + ": " + curr_color + line + "\n";

    if (!color_code.empty())
      curr_color = color_code; // remember last color of this line
  }
  message = colored_message;
}

std::list<std::string>
getFilesInDirs(const std::list<std::string> & directory_list)
{
  std::list<std::string> files;

  for (const auto & dir_name : directory_list)
  {
    tinydir_dir dir;
    dir.has_next = 0; // Avoid a garbage value in has_next (clang StaticAnalysis)
    tinydir_open(&dir, dir_name.c_str());

    while (dir.has_next)
    {
      tinydir_file file;
      file.is_dir = 0; // Avoid a garbage value in is_dir (clang StaticAnalysis)
      tinydir_readfile(&dir, &file);

      if (!file.is_dir)
        files.push_back(dir_name + "/" + file.name);

      tinydir_next(&dir);
    }

    tinydir_close(&dir);
  }

  return files;
}

std::string
getRecoveryFileBase(const std::list<std::string> & checkpoint_files)
{
  // Create storage for newest restart files
  // Note that these might have the same modification time if the simulation was fast.
  // In that case we're going to save all of the "newest" files and sort it out momentarily
  time_t newest_time = 0;
  std::list<std::string> newest_restart_files;

  // Loop through all possible files and store the newest
  for (const auto & cp_file : checkpoint_files)
  {
    // Only look at the main checkpoint file, not the mesh, or restartable data files
    if (hasExtension(cp_file, "xdr"))
    {
      struct stat stats;
      stat(cp_file.c_str(), &stats);

      time_t mod_time = stats.st_mtime;
      if (mod_time > newest_time)
      {
        newest_restart_files.clear(); // If the modification time is greater, clear the list
        newest_time = mod_time;
      }

      if (mod_time == newest_time)
        newest_restart_files.push_back(cp_file);
    }
  }

  // Loop through all of the newest files according the number in the file name
  int max_file_num = -1;
  std::string max_base;
  pcrecpp::RE re_base_and_file_num(
      "(.*?(\\d+))\\..*"); // Will pull out the full base and the file number simultaneously

  // Now, out of the newest files find the one with the largest number in it
  for (const auto & res_file : newest_restart_files)
  {
    std::string the_base;
    int file_num = 0;

    re_base_and_file_num.FullMatch(res_file, &the_base, &file_num);

    if (file_num > max_file_num)
    {
      max_file_num = file_num;
      max_base = the_base;
    }
  }

  // Error if nothing was located
  if (max_file_num == -1)
    max_base.clear();

  return max_base;
}

bool
wildCardMatch(std::string name, std::string search_string)
{
  // Assume that an empty string matches anything
  if (search_string == "")
    return true;

  // transform to lower for case insenstive matching
  std::transform(name.begin(), name.end(), name.begin(), (int (*)(int))std::toupper);
  std::transform(search_string.begin(),
                 search_string.end(),
                 search_string.begin(),
                 (int (*)(int))std::toupper);

  // exact match!
  if (search_string.find("*") == std::string::npos)
    return search_string == name;

  // wildcard
  std::vector<std::string> tokens;
  MooseUtils::tokenize(search_string, tokens, 1, "*");

  size_t pos = 0;
  for (unsigned int i = 0; i < tokens.size() && pos != std::string::npos; ++i)
  {
    pos = name.find(tokens[i], pos);
    // See if we have a leading wildcard
    if (search_string[0] != '*' && i == 0 && pos != 0)
      return false;
  }

  if (pos != std::string::npos && tokens.size() > 0)
  {
    // Now see if we have a trailing wildcard
    size_t last_token_length = tokens.back().length();
    if (*search_string.rbegin() == '*' || pos == name.size() - last_token_length)
      return true;
    else
      return false;
  }
  else
    return false;
}

MultiMooseEnum
createExecuteOnEnum(int n, ...)
{
  // Define the default execute_on flags
  MultiMooseEnum exec_enum;
  std::set<ExecFlagType> flags = {EXEC_NONE,
                                  EXEC_INITIAL,
                                  EXEC_LINEAR,
                                  EXEC_NONLINEAR,
                                  EXEC_TIMESTEP_END,
                                  EXEC_TIMESTEP_BEGIN,
                                  EXEC_CUSTOM,
                                  EXEC_SUBDOMAIN};
  for (const ExecFlagType & flag : flags)
  {
    auto iter = getExecuteOnFlag(flag);
    exec_enum.addEnumerationName(iter->second, iter->first);
  }

  // Add the default flags
  va_list args;
  va_start(args, n);
  for (int i = 0; i < n; i++)
  {
    const ExecFlagType & flag = va_arg(args, ExecFlagType);
    const auto iter = Moose::execute_flags.find(flag);
    if (iter == Moose::execute_flags.end())
      mooseError("Unknown flag value of ", flag, ", the flag is likely not registered.");
    exec_enum.push_back(iter->second);
  }
  va_end(args);

  return exec_enum;
}

std::string
getExecuteOnEnumDocString(const MultiMooseEnum & exec_enum)
{
  std::string doc("The list of flag(s) indicating when this object should be executed, the "
                  "available optoins include \'");
  for (const std::string & name : exec_enum.getNames())
    doc += name + "', '";
  doc.erase(doc.end() - 4, doc.end());
  doc += "').";
  return doc;
}

void
addExecuteOnFlags(InputParameters & params, int n, ...)
{
  MultiMooseEnum & exec_enum = getExecuteOnEnum(params);
  va_list args;
  va_start(args, n);
  for (int i = 0; i < n; i++)
  {
    const auto iter = getExecuteOnFlag(va_arg(args, ExecFlagType));
    exec_enum.addEnumerationName(iter->second, iter->first);
  }
  va_end(args);
  params.setDocString("execute_on", getExecuteOnEnumDocString(exec_enum));
}

void
removeExecuteOnFlags(InputParameters & params, int n, ...)
{
  MultiMooseEnum & exec_enum = getExecuteOnEnum(params);
  va_list args;
  va_start(args, n);
  for (int i = 0; i < n; i++)
  {
    const auto iter = getExecuteOnFlag(va_arg(args, ExecFlagType));
    exec_enum.removeEnumerationName(iter->second);
  }
  va_end(args);
  params.setDocString("execute_on", getExecuteOnEnumDocString(exec_enum));
}

void
setExecuteOnFlags(InputParameters & params, int n, ...)
{
  MultiMooseEnum & exec_enum = getExecuteOnEnum(params);
  exec_enum.clear();
  va_list args;
  va_start(args, n);
  for (int i = 0; i < n; i++)
  {
    const auto & iter = getExecuteOnFlag(va_arg(args, ExecFlagType));
    exec_enum.push_back(iter->second);
  }
  va_end(args);

  // Re-apply the parameter to maintain the "set by user" status
  params.addParam<MultiMooseEnum>("execute_on", exec_enum, getExecuteOnEnumDocString(exec_enum));
}

std::map<ExecFlagType, std::string>::const_iterator
getExecuteOnFlag(const ExecFlagType & flag)
{
  const auto iter = Moose::execute_flags.find(flag);
  if (iter == Moose::execute_flags.end())
    mooseError("Unknown flag value of ", flag, ", the flag is likely not registered.");
  return iter;
}

MultiMooseEnum &
getExecuteOnEnum(InputParameters & parameters)
{
  if (!parameters.isParamValid("execute_on"))
    mooseError("Cannot add execute flags, the InputParameters do not have 'execute_on'.");
  return parameters.template set<MultiMooseEnum>("execute_on");
}

} // MooseUtils namespace
