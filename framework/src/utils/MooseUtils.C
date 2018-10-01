//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MooseUtils.h"
#include "MooseError.h"
#include "MaterialProperty.h"
#include "MultiMooseEnum.h"
#include "InputParameters.h"
#include "ExecFlagEnum.h"

#include "libmesh/elem.h"

// External includes
#include "pcrecpp.h"
#include "tinydir.h"

// C++ includes
#include <iostream>
#include <fstream>
#include <istream>
#include <iterator>
#include <ctime>

// System includes
#include <sys/stat.h>
#include <numeric>
#include <unistd.h>

std::string getLatestCheckpointFileHelper(const std::list<std::string> & checkpoint_files,
                                          const std::vector<std::string> extensions,
                                          bool keep_extension);

namespace MooseUtils
{

// this implementation is copied from
// https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C.2B.2B
int
levenshteinDist(const std::string & s1, const std::string & s2)
{
  // To change the type this function manipulates and returns, change
  // the return type and the types of the two variables below.
  auto s1len = s1.size();
  auto s2len = s2.size();

  auto column_start = (decltype(s1len))1;

  auto column = new decltype(s1len)[s1len + 1];
  std::iota(column + column_start, column + s1len + 1, column_start);

  for (auto x = column_start; x <= s2len; x++)
  {
    column[0] = x;
    auto last_diagonal = x - column_start;
    for (auto y = column_start; y <= s1len; y++)
    {
      auto old_diagonal = column[y];
      auto possibilities = {
          column[y] + 1, column[y - 1] + 1, last_diagonal + (s1[y - 1] == s2[x - 1] ? 0 : 1)};
      column[y] = std::min(possibilities);
      last_diagonal = old_diagonal;
    }
  }
  auto result = column[s1len];
  delete[] column;
  return result;
}

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
trim(const std::string & str, const std::string & white_space)
{
  const auto begin = str.find_first_not_of(white_space);
  if (begin == std::string::npos)
    return ""; // no content
  const auto end = str.find_last_not_of(white_space);
  return str.substr(begin, end - begin + 1);
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
pathExists(const std::string & path)
{
  struct stat buffer;
  return (stat(path.c_str(), &buffer) == 0);
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
parallelBarrierNotify(const Parallel::Communicator & comm, bool messaging)
{
  processor_id_type slave_processor_id;

  if (comm.rank() == 0)
  {
    // The master process is already through, so report it
    if (messaging)
      Moose::out << "Jobs complete: 1/" << comm.size() << (1 == comm.size() ? "\n" : "\r")
                 << std::flush;
    for (unsigned int i = 2; i <= comm.size(); ++i)
    {
      comm.receive(MPI_ANY_SOURCE, slave_processor_id);
      if (messaging)
        Moose::out << "Jobs complete: " << i << "/" << comm.size()
                   << (i == comm.size() ? "\n" : "\r") << std::flush;
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
serialBegin(const libMesh::Parallel::Communicator & comm, bool warn)
{
  // unless we are the first processor...
  if (comm.rank() > 0)
  {
    // ...wait for the previous processor to finish
    int dummy = 0;
    comm.receive(comm.rank() - 1, dummy);
  }
  else if (warn)
    mooseWarning("Entering serial execution block (use only for debugging)");
}

void
serialEnd(const libMesh::Parallel::Communicator & comm, bool warn)
{
  // unless we are the last processor...
  if (comm.rank() + 1 < comm.size())
  {
    // ...notify the next processor of its turn
    int dummy = 0;
    comm.send(comm.rank() + 1, dummy);
  }

  comm.barrier();
  if (comm.rank() == 0 && warn)
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

std::string
stripExtension(const std::string & s)
{
  auto pos = s.rfind(".");
  if (pos != std::string::npos)
    return s.substr(0, pos);
  return s;
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
  pcrecpp::RE("(?!^)(?<![A-Z])([A-Z]+)").GlobalReplace("_\\1", &replaced);

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

std::string
hostname()
{
  // This is from: https://stackoverflow.com/a/505546
  char hostname[1024];
  hostname[1023] = '\0';

  auto failure = gethostname(hostname, 1023);

  if (failure)
    mooseError("Failed to retrieve hostname!");

  return hostname;
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
listDir(const std::string path, bool files_only)
{
  std::list<std::string> files;

  tinydir_dir dir;
  dir.has_next = 0; // Avoid a garbage value in has_next (clang StaticAnalysis)
  tinydir_open(&dir, path.c_str());

  while (dir.has_next)
  {
    tinydir_file file;
    file.is_dir = 0; // Avoid a garbage value in is_dir (clang StaticAnalysis)
    tinydir_readfile(&dir, &file);

    if (!files_only || !file.is_dir)
      files.push_back(path + "/" + file.name);

    tinydir_next(&dir);
  }

  tinydir_close(&dir);

  return files;
}

std::list<std::string>
getFilesInDirs(const std::list<std::string> & directory_list)
{
  std::list<std::string> files;

  for (const auto & dir_name : directory_list)
    files.splice(files.end(), listDir(dir_name, true));

  return files;
}

std::string
getLatestMeshCheckpointFile(const std::list<std::string> & checkpoint_files)
{
  const static std::vector<std::string> extensions{"cpr"};

  return getLatestCheckpointFileHelper(checkpoint_files, extensions, true);
}

std::string
getLatestAppCheckpointFileBase(const std::list<std::string> & checkpoint_files)
{
  const static std::vector<std::string> extensions{"xda", "xdr"};

  return getLatestCheckpointFileHelper(checkpoint_files, extensions, false);
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

template <typename T>
T
convertStringToInt(const std::string & str, bool throw_on_failure)
{
  T val;

  // Let's try to read a double and see if we can cast it to an int
  // This would be the case for scientific notation
  long double double_val;
  std::stringstream double_ss(str);

  if ((double_ss >> double_val).fail() || !double_ss.eof())
  {
    std::string msg =
        std::string("Unable to convert '") + str + "' to type " + demangle(typeid(T).name());

    if (throw_on_failure)
      throw std::invalid_argument(msg);
    else
      mooseError(msg);
  }

  // Check to see if it's an integer (and within range of an integer
  if (double_val == static_cast<T>(double_val))
    val = double_val;
  else // Still failure
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
short int
convert<short int>(const std::string & str, bool throw_on_failure)
{
  return convertStringToInt<short int>(str, throw_on_failure);
}

template <>
unsigned short int
convert<unsigned short int>(const std::string & str, bool throw_on_failure)
{
  return convertStringToInt<unsigned short int>(str, throw_on_failure);
}

template <>
int
convert<int>(const std::string & str, bool throw_on_failure)
{
  return convertStringToInt<int>(str, throw_on_failure);
}

template <>
unsigned int
convert<unsigned int>(const std::string & str, bool throw_on_failure)
{
  return convertStringToInt<unsigned int>(str, throw_on_failure);
}

template <>
long int
convert<long int>(const std::string & str, bool throw_on_failure)
{
  return convertStringToInt<long int>(str, throw_on_failure);
}

template <>
unsigned long int
convert<unsigned long int>(const std::string & str, bool throw_on_failure)
{
  return convertStringToInt<unsigned long int>(str, throw_on_failure);
}

template <>
long long int
convert<long long int>(const std::string & str, bool throw_on_failure)
{
  return convertStringToInt<long long int>(str, throw_on_failure);
}

template <>
unsigned long long int
convert<unsigned long long int>(const std::string & str, bool throw_on_failure)
{
  return convertStringToInt<unsigned long long int>(str, throw_on_failure);
}

std::string
toUpper(const std::string & name)
{
  std::string upper(name);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
  return upper;
}

std::string
toLower(const std::string & name)
{
  std::string lower(name);
  std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
  return lower;
}

ExecFlagEnum
getDefaultExecFlagEnum()
{
  ExecFlagEnum exec_enum = ExecFlagEnum();
  exec_enum.addAvailableFlags(EXEC_NONE,
                              EXEC_INITIAL,
                              EXEC_LINEAR,
                              EXEC_NONLINEAR,
                              EXEC_TIMESTEP_END,
                              EXEC_TIMESTEP_BEGIN,
                              EXEC_FINAL,
                              EXEC_CUSTOM);
  return exec_enum;
}

int
stringToInteger(const std::string & input, bool throw_on_failure)
{
  return convert<int>(input, throw_on_failure);
}

} // MooseUtils namespace

std::string
getLatestCheckpointFileHelper(const std::list<std::string> & checkpoint_files,
                              const std::vector<std::string> extensions,
                              bool keep_extension)
{
  // Create storage for newest restart files
  // Note that these might have the same modification time if the simulation was fast.
  // In that case we're going to save all of the "newest" files and sort it out momentarily
  std::time_t newest_time = 0;
  std::list<std::string> newest_restart_files;

  // Loop through all possible files and store the newest
  for (const auto & cp_file : checkpoint_files)
  {
    if (find_if(extensions.begin(), extensions.end(), [cp_file](const std::string & ext) {
          return MooseUtils::hasExtension(cp_file, ext);
        }) != extensions.end())
    {
      struct stat stats;
      stat(cp_file.c_str(), &stats);

      std::time_t mod_time = stats.st_mtime;
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
  std::string max_file;

  pcrecpp::RE re_file_num(".*?(\\d+)(?:_mesh)?$"); // Pull out the embedded number from the file

  // Now, out of the newest files find the one with the largest number in it
  for (const auto & res_file : newest_restart_files)
  {
    auto dot_pos = res_file.find_last_of(".");
    auto the_base = res_file.substr(0, dot_pos);
    int file_num = 0;

    re_file_num.FullMatch(the_base, &file_num);

    if (file_num > max_file_num)
    {
      max_file_num = file_num;
      max_base = the_base;
      max_file = res_file;
    }
  }

  // Error if nothing was located
  if (max_file_num == -1)
  {
    max_base.clear();
    max_file.clear();
  }

  return keep_extension ? max_file : max_base;
}
