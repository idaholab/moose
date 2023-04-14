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
#include "InfixIterator.h"
#include "Registry.h"
#include "MortarConstraintBase.h"
#include "MortarNodalAuxKernel.h"
#include "ExecFlagRegistry.h"

#include "libmesh/utility.h"
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

#include "petscsys.h"

#ifdef __WIN32__
#include <windows.h>
#include <winbase.h>
#include <fileapi.h>
#endif

std::string getLatestCheckpointFileHelper(const std::list<std::string> & checkpoint_files,
                                          const std::vector<std::string> extensions,
                                          bool keep_extension);

namespace MooseUtils
{
std::string
pathjoin(const std::string & s)
{
  return s;
}

std::string
runTestsExecutable()
{
  auto build_loc = pathjoin(Moose::getExecutablePath(), "run_tests");
  if (pathExists(build_loc) && checkFileReadable(build_loc))
    return build_loc;
  // TODO: maybe no path prefix - just moose_test_runner here?
  return pathjoin(Moose::getExecutablePath(), "moose_test_runner");
}

std::string
findTestRoot()
{
  std::string path = ".";
  for (int i = 0; i < 5; i++)
  {
    auto testroot = pathjoin(path, "testroot");
    if (pathExists(testroot) && checkFileReadable(testroot))
      return testroot;
    path += "/..";
  }
  return "";
}

bool
parsesToReal(const std::string & input)
{
  std::istringstream ss(input);
  Real real_value;
  if (ss >> real_value && ss.eof())
    return true;
  return false;
}

std::string
installedInputsDir(const std::string & app_name,
                   const std::string & dir_name,
                   const std::string & extra_error_msg)
{
  // See moose.mk for a detailed explanation of the assumed installed application
  // layout. Installed inputs are expected to be installed in "share/<app_name>/<folder>".
  // The binary, which has a defined location will be in "bin", a peer directory to "share".
  std::string installed_path =
      pathjoin(Moose::getExecutablePath(), "..", "share", app_name, dir_name);

  auto test_root = pathjoin(installed_path, "testroot");
  if (!pathExists(installed_path))
    mooseError("Couldn't locate any installed inputs to copy in path: ",
               installed_path,
               '\n',
               extra_error_msg);

  checkFileReadable(test_root);
  return installed_path;
}

std::string
docsDir(const std::string & app_name)
{
  // See moose.mk for a detailed explanation of the assumed installed application
  // layout. Installed docs are expected to be installed in "share/<app_name>/doc".
  // The binary, which has a defined location will be in "bin", a peer directory to "share".
  std::string installed_path = pathjoin(Moose::getExecutablePath(), "..", "share", app_name, "doc");

  auto docfile = pathjoin(installed_path, "css", "moose.css");
  if (pathExists(docfile) && checkFileReadable(docfile))
    return installed_path;
  return "";
}

std::string
replaceAll(std::string str, const std::string & from, const std::string & to)
{
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos)
  {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
  }
  return str;
}

std::string
convertLatestCheckpoint(std::string orig, bool base_only)
{
  auto slash_pos = orig.find_last_of("/");
  auto path = orig.substr(0, slash_pos);
  auto file = orig.substr(slash_pos + 1);
  if (file != "LATEST")
    return orig;

  auto converted = MooseUtils::getLatestAppCheckpointFileBase(MooseUtils::listDir(path));
  if (!base_only)
    converted = MooseUtils::getLatestMeshCheckpointFile(MooseUtils::listDir(path));
  else if (converted.empty())
    mooseError("Unable to find suitable recovery file!");
  return converted;
}

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
pathIsDirectory(const std::string & path)
{
  struct stat buffer;
  // stat call fails?
  if (stat(path.c_str(), &buffer))
    return false;
  return S_IFDIR & buffer.st_mode;
}

bool
checkFileReadable(const std::string & filename,
                  bool check_line_endings,
                  bool throw_on_unreadable,
                  bool check_for_git_lfs_pointer)
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

  if (check_for_git_lfs_pointer && checkForGitLFSPointer(in))
    mooseError(filename + " appears to be a Git-LFS pointer. Make sure you have \"git-lfs\" "
                          "installed so that you may pull this file.");
  in.close();

  return true;
}

bool
checkForGitLFSPointer(std::ifstream & file)
{
  mooseAssert(file.is_open(), "Passed in file handle is not open");

  std::string line;

  // git-lfs pointer files contain several name value pairs. The specification states that the
  // first name/value pair must be "version {url}". We'll do a simplified check for that.
  file.seekg(0);
  std::getline(file, line);
  if (line.find("version https://") != std::string::npos)
    return true;
  else
    return false;
}

bool
checkFileWriteable(const std::string & filename, bool throw_on_unwritable)
{
  std::ofstream out(filename.c_str(), std::ios_base::app);
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
  processor_id_type secondary_processor_id;

  if (messaging)
    Moose::out << "Waiting For Other Processors To Finish" << std::endl;
  if (comm.rank() == 0)
  {
    // The primary process is already through, so report it
    if (messaging)
      Moose::out << "Jobs complete: 1/" << comm.size() << (1 == comm.size() ? "\n" : "\r")
                 << std::flush;
    for (unsigned int i = 2; i <= comm.size(); ++i)
    {
      comm.receive(MPI_ANY_SOURCE, secondary_processor_id);
      if (messaging)
        Moose::out << "Jobs complete: " << i << "/" << comm.size()
                   << (i == comm.size() ? "\n" : "\r") << std::flush;
    }
  }
  else
  {
    secondary_processor_id = comm.rank();
    comm.send(0, secondary_processor_id);
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
getCurrentWorkingDir()
{
  // Note: At the time of creating this method, our minimum compiler still
  // does not support <filesystem>. Additionally, the inclusion of that header
  // requires an additional library to be linked so for now, we'll just
  // use the Unix standard library to get us the cwd().
  constexpr unsigned int BUF_SIZE = 1024;
  char buffer[BUF_SIZE];

  return getcwd(buffer, BUF_SIZE) != nullptr ? buffer : "";
}

void
makedirs(const std::string & dir_name, bool throw_on_failure)
{
  // split path into directories with delimiter '/'
  std::vector<std::string> split_dir_names;
  MooseUtils::tokenize(dir_name, split_dir_names);

  auto n = split_dir_names.size();

  // remove '.' and '..' when possible
  auto i = n;
  i = 0;
  while (i != n)
  {
    if (split_dir_names[i] == ".")
    {
      for (auto j = i + 1; j < n; ++j)
        split_dir_names[j - 1] = split_dir_names[j];
      --n;
    }
    else if (i > 0 && split_dir_names[i] == ".." && split_dir_names[i - 1] != "..")
    {
      for (auto j = i + 1; j < n; ++j)
        split_dir_names[j - 2] = split_dir_names[j];
      n -= 2;
      --i;
    }
    else
      ++i;
  }
  if (n == 0)
    return;

  split_dir_names.resize(n);

  // start creating directories recursively
  std::string cur_dir = dir_name[0] == '/' ? "" : ".";
  for (auto & dir : split_dir_names)
  {
    cur_dir += "/" + dir;

    if (!pathExists(cur_dir))
    {
      auto code = Utility::mkdir(cur_dir.c_str());
      if (code != 0)
      {
        std::string msg = "Failed creating directory " + dir_name;
        if (throw_on_failure)
          throw std::invalid_argument(msg);
        else
          mooseError(msg);
      }
    }
  }
}

void
removedirs(const std::string & dir_name, bool throw_on_failure)
{
  // split path into directories with delimiter '/'
  std::vector<std::string> split_dir_names;
  MooseUtils::tokenize(dir_name, split_dir_names);

  auto n = split_dir_names.size();

  // remove '.' and '..' when possible
  auto i = n;
  i = 0;
  while (i != n)
  {
    if (split_dir_names[i] == ".")
    {
      for (auto j = i + 1; j < n; ++j)
        split_dir_names[j - 1] = split_dir_names[j];
      --n;
    }
    else if (i > 0 && split_dir_names[i] == ".." && split_dir_names[i - 1] != "..")
    {
      for (auto j = i + 1; j < n; ++j)
        split_dir_names[j - 2] = split_dir_names[j];
      n -= 2;
      --i;
    }
    else
      ++i;
  }
  if (n == 0)
    return;

  split_dir_names.resize(n);

  // start removing directories recursively
  std::string base_dir = dir_name[0] == '/' ? "" : ".";
  for (i = n; i > 0; --i)
  {
    std::string cur_dir = base_dir;
    auto j = i;
    for (j = 0; j < i; ++j)
      cur_dir += "/" + split_dir_names[j];

    // listDir should return at least '.' and '..'
    if (pathExists(cur_dir) && listDir(cur_dir).size() == 2)
    {
      auto code = rmdir(cur_dir.c_str());
      if (code != 0)
      {
        std::string msg = "Failed removing directory " + dir_name;
        if (throw_on_failure)
          throw std::invalid_argument(msg);
        else
          mooseError(msg);
      }
    }
    else
      // stop removing
      break;
  }
}

std::string
camelCaseToUnderscore(const std::string & camel_case_name)
{
  std::string replaced = camel_case_name;
  // Put underscores in front of each contiguous set of capital letters
  pcrecpp::RE("(?!^)(?<![A-Z_])([A-Z]+)").GlobalReplace("_\\1", &replaced);

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
  char hostname[1024];
  hostname[1023] = '\0';
#ifndef __WIN32__
  if (gethostname(hostname, 1023))
    mooseError("Failed to retrieve hostname!");
#else
  DWORD dwSize = sizeof(hostname);
  if (!GetComputerNameEx(ComputerNamePhysicalDnsHostname, hostname, &dwSize))
    mooseError("Failed to retrieve hostname!");
#endif
  return hostname;
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

  Moose::out << std::flush;
}

std::string &
removeColor(std::string & msg)
{
  pcrecpp::RE re("(\\33\\[3[0-7]m))", pcrecpp::DOTALL());
  re.GlobalReplace(std::string(""), &msg);
  return msg;
}

void
addLineBreaks(std::string & message,
              unsigned int line_width /*= ConsoleUtils::console_line_length*/)
{
  for (auto i : make_range(int(message.length() / line_width)))
    message.insert((i + 1) * (line_width + 2) - 2, "\n");
}

void
indentMessage(const std::string & prefix,
              std::string & message,
              const char * color /*= COLOR_CYAN*/,
              bool indent_first_line,
              const std::string & post_prefix)
{
  // First we need to see if the message we need to indent (with color) also contains color codes
  // that span lines.
  // The code matches all of the XTERM constants (see XTermConstants.h). If it does, then we'll work
  // on formatting
  // each colored multiline chunk one at a time with the right codes.
  std::string colored_message;
  std::string curr_color = COLOR_DEFAULT; // tracks last color code before newline
  std::string line, color_code;

  bool ends_in_newline = message.empty() ? true : message.back() == '\n';

  bool first = true;

  std::istringstream iss(message);
  for (std::string line; std::getline(iss, line);) // loop over each line
  {
    const static pcrecpp::RE match_color(".*(\\33\\[3\\dm)((?!\\33\\[3\\d)[^\n])*");
    pcrecpp::StringPiece line_piece(line);
    match_color.FindAndConsume(&line_piece, &color_code);

    if (!first || indent_first_line)
      colored_message += color + prefix + post_prefix + curr_color;

    colored_message += line;

    // Only add a newline to the last line if it had one to begin with!
    if (!iss.eof() || ends_in_newline)
      colored_message += "\n";

    if (!color_code.empty())
      curr_color = color_code; // remember last color of this line

    first = false;
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

bool
globCompare(const std::string & candidate,
            const std::string & pattern,
            std::size_t c,
            std::size_t p)
{
  if (p == pattern.size())
    return c == candidate.size();

  if (pattern[p] == '*')
  {
    for (; c < candidate.size(); ++c)
      if (globCompare(candidate, pattern, c, p + 1))
        return true;
    return globCompare(candidate, pattern, c, p + 1);
  }

  if (pattern[p] != '?' && pattern[p] != candidate[c])
    return false;

  return globCompare(candidate, pattern, c + 1, p + 1);
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
  double_ss >> double_val;

  // on arm64 the long double does not have sufficient precission
  bool use_int = false;
  std::stringstream int_ss(str);
  if (!(int_ss >> val).fail() && int_ss.eof())
    use_int = true;

  if (double_ss.fail() || !double_ss.eof())
  {
    std::string msg =
        std::string("Unable to convert '") + str + "' to type " + demangle(typeid(T).name());

    if (throw_on_failure)
      throw std::invalid_argument(msg);
    else
      mooseError(msg);
  }

  // Check to see if it's an integer (and within range of an integer)
  if (double_val == static_cast<T>(double_val))
    return use_int ? val : static_cast<T>(double_val);

  // Still failure
  std::string msg =
      std::string("Unable to convert '") + str + "' to type " + demangle(typeid(T).name());

  if (throw_on_failure)
    throw std::invalid_argument(msg);
  else
    mooseError(msg);
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
  return moose::internal::ExecFlagRegistry::getExecFlagRegistry().getDefaultFlags();
}

int
stringToInteger(const std::string & input, bool throw_on_failure)
{
  return convert<int>(input, throw_on_failure);
}

void
linearPartitionItems(dof_id_type num_items,
                     dof_id_type num_chunks,
                     dof_id_type chunk_id,
                     dof_id_type & num_local_items,
                     dof_id_type & local_items_begin,
                     dof_id_type & local_items_end)
{
  auto global_num_local_items = num_items / num_chunks;

  num_local_items = global_num_local_items;

  auto leftovers = num_items % num_chunks;

  if (chunk_id < leftovers)
  {
    num_local_items++;
    local_items_begin = num_local_items * chunk_id;
  }
  else
    local_items_begin =
        (global_num_local_items + 1) * leftovers + global_num_local_items * (chunk_id - leftovers);

  local_items_end = local_items_begin + num_local_items;
}

processor_id_type
linearPartitionChunk(dof_id_type num_items, dof_id_type num_chunks, dof_id_type item_id)
{
  auto global_num_local_items = num_items / num_chunks;

  auto leftovers = num_items % num_chunks;

  auto first_item_past_first_part = leftovers * (global_num_local_items + 1);

  // Is it in the first section (that gets an extra item)
  if (item_id < first_item_past_first_part)
    return item_id / (global_num_local_items + 1);
  else
  {
    auto new_item_id = item_id - first_item_past_first_part;

    // First chunk after the first section + the number of chunks after that
    return leftovers + (new_item_id / global_num_local_items);
  }
}

std::vector<std::string>
split(const std::string & str, const std::string & delimiter, std::size_t max_count)
{
  std::vector<std::string> output;
  std::size_t count = 0;
  size_t prev = 0, pos = 0;
  do
  {
    pos = str.find(delimiter, prev);
    output.push_back(str.substr(prev, pos - prev));
    prev = pos + delimiter.length();
    count += 1;
  } while (pos != std::string::npos && count < max_count);

  if (pos != std::string::npos)
    output.push_back(str.substr(prev));

  return output;
}

std::vector<std::string>
rsplit(const std::string & str, const std::string & delimiter, std::size_t max_count)
{
  std::vector<std::string> output;
  std::size_t count = 0;
  size_t prev = str.length(), pos = str.length();
  do
  {
    pos = str.rfind(delimiter, prev);
    output.insert(output.begin(), str.substr(pos + delimiter.length(), prev - pos));
    prev = pos - delimiter.length();
    count += 1;
  } while (pos != std::string::npos && pos > 0 && count < max_count);

  if (pos != std::string::npos)
    output.insert(output.begin(), str.substr(0, pos));

  return output;
}

void
createSymlink(const std::string & target, const std::string & link)
{
  clearSymlink(link);
#ifndef __WIN32__
  auto err = symlink(target.c_str(), link.c_str());
#else
  auto err = CreateSymbolicLink(target.c_str(), link.c_str(), 0);
#endif
  if (err)
    mooseError("Failed to create symbolic link (via 'symlink') from ", target, " to ", link);
}

void
clearSymlink(const std::string & link)
{
#ifndef __WIN32__
  struct stat sbuf;
  if (lstat(link.c_str(), &sbuf) == 0)
  {
    auto err = unlink(link.c_str());
    if (err != 0)
      mooseError("Failed to remove symbolic link (via 'unlink') to ", link);
  }
#else
  auto attr = GetFileAttributesA(link.c_str());
  if (attr != INVALID_FILE_ATTRIBUTES)
  {
    auto err = _unlink(link.c_str());
    if (err != 0)
      mooseError("Failed to remove link/file (via '_unlink') to ", link);
  }
#endif
}

std::size_t
fileSize(const std::string & filename)
{
#ifndef __WIN32__
  struct stat buffer;
  if (!stat(filename.c_str(), &buffer))
    return buffer.st_size;
#else
  HANDLE hFile = CreateFile(filename.c_str(),
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
  if (hFile == INVALID_HANDLE_VALUE)
    return 0;

  LARGE_INTEGER size;
  if (GetFileSizeEx(hFile, &size))
  {
    CloseHandle(hFile);
    return size.QuadPart;
  }

  CloseHandle(hFile);
#endif
  return 0;
}

std::string
realpath(const std::string & path)
{
  char dummy[PETSC_MAX_PATH_LEN];
  if (PetscGetFullPath(path.c_str(), dummy, sizeof(dummy)))
    mooseError("Failed to get real path for ", path);
  return dummy;
}

std::string
relativepath(const std::string & path, const std::string & start)
{
  std::vector<std::string> vecpath;
  std::vector<std::string> vecstart;
  size_t index_size;
  unsigned int same_size(0);

  vecpath = split(path, "/");
  vecstart = split(realpath(start), "/");
  if (vecstart.size() < vecpath.size())
    index_size = vecstart.size();
  else
    index_size = vecpath.size();

  for (unsigned int i = 0; i < index_size; ++i)
  {
    if (vecstart[i] != vecpath[i])
    {
      same_size = i;
      break;
    }
  }

  std::string relative_path("");
  for (unsigned int i = 0; i < (vecstart.size() - same_size); ++i)
    relative_path += "../";

  for (unsigned int i = same_size; i < vecpath.size(); ++i)
  {
    relative_path += vecpath[i];
    if (i < (vecpath.size() - 1))
      relative_path += "/";
  }

  return relative_path;
}

BoundingBox
buildBoundingBox(const Point & p1, const Point & p2)
{
  BoundingBox bb;
  bb.union_with(p1);
  bb.union_with(p2);
  return bb;
}

std::string
prettyCppType(const std::string & cpp_type)
{
  // On mac many of the std:: classes are inline namespaced with __1
  // On linux std::string can be inline namespaced with __cxx11
  std::string s = cpp_type;
  // Remove all spaces surrounding a >
  pcrecpp::RE("\\s(?=>)").GlobalReplace("", &s);
  pcrecpp::RE("std::__\\w+::").GlobalReplace("std::", &s);
  // It would be nice if std::string actually looked normal
  pcrecpp::RE("\\s*std::basic_string<char, std::char_traits<char>, std::allocator<char>>\\s*")
      .GlobalReplace("std::string", &s);
  // It would be nice if std::vector looked normal
  pcrecpp::RE r("std::vector<([[:print:]]+),\\s?std::allocator<\\s?\\1\\s?>\\s?>");
  r.GlobalReplace("std::vector<\\1>", &s);
  // Do it again for nested vectors
  r.GlobalReplace("std::vector<\\1>", &s);
  return s;
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
    if (find_if(extensions.begin(),
                extensions.end(),
                [cp_file](const std::string & ext)
                { return MooseUtils::hasExtension(cp_file, ext); }) != extensions.end())
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

void
removeSubstring(std::string & main, const std::string & sub)
{
  std::string::size_type n = sub.length();
  for (std::string::size_type i = main.find(sub); i != std::string::npos; i = main.find(sub))
    main.erase(i, n);
}

std::string
removeSubstring(const std::string & main, const std::string & sub)
{
  std::string copy_main = main;
  std::string::size_type n = sub.length();
  for (std::string::size_type i = copy_main.find(sub); i != std::string::npos;
       i = copy_main.find(sub))
    copy_main.erase(i, n);
  return copy_main;
}
