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
    for (size_t pos=0; (pos=str.find(it.first, pos)) != std::string::npos; pos+=it.second.size())
      str.replace(pos, 1, it.second);
}


std::string
trim(std::string str, const std::string & white_space)
{
  std::string r = str.erase(str.find_last_not_of(white_space)+1);
  return r.erase(0,r.find_first_not_of(white_space));
}

bool pathContains(const std::string & expression,
                  const std::string & string_to_find,
                  const std::string & delims)
{
  std::vector<std::string> elements;
  tokenize(expression, elements, 0, delims);

  std::vector<std::string>::iterator found_it = std::find(elements.begin(), elements.end(), string_to_find);
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
      mooseError2((std::string("Unable to open file \"") + filename
                  + std::string("\". Check to make sure that it exists and that you have read permission.")).c_str());
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
        mooseError2(filename + " contains Windows(DOS) line endings which are not supported.");
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
      mooseError2((std::string("Unable to open file \"") + filename
                  + std::string("\". Check to make sure that it exists and that you have write permission.")).c_str());
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
    Moose::out << "Jobs complete: 1/" << comm.size() << (1 == comm.size() ? "\n" : "\r") << std::flush;
    for (unsigned int i=2; i<=comm.size(); ++i)
    {
      comm.receive(MPI_ANY_SOURCE, slave_processor_id);
      Moose::out << "Jobs complete: " << i << "/" << comm.size() << (i == comm.size() ? "\n" : "\r") << std::flush;
    }
  }
  else
  {
    slave_processor_id = comm.rank();
    comm.send(0, slave_processor_id);
  }

  comm.barrier();
}

void serialBegin(const libMesh::Parallel::Communicator & comm)
{
  // unless we are the first processor...
  if (comm.rank() > 0)
  {
    // ...wait for the previous processor to finish
    int dummy = 0;
    comm.receive(comm.rank() - 1, dummy);
  }
  else
    mooseWarning2("Entering serial execution block (use only for debugging)");
}

void serialEnd(const libMesh::Parallel::Communicator & comm)
{
  // unless we are the last processor...
  if (comm.rank() + 1 < comm.size())
  {
    // ...notify the next processor of its turn
    int dummy = 0;
    comm.send(comm.rank() + 1, dummy);
  }
  else
    mooseWarning2("Leaving serial execution block (use only for debugging)");
}

bool
hasExtension(const std::string & filename, std::string ext, bool strip_exodus_ext)
{
  // Extract the extension, w/o the '.'
  std::string file_ext;
  if (strip_exodus_ext)
  {
    pcrecpp::RE re(".*\\.([^\\.]*?)(?:-s\\d+)?\\s*$"); // capture the complete extension, ignoring -s*
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
    mooseError2("Invalid full file name: ", full_file);

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
    file = full_file.substr(found+1);
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
  return (absoluteFuzzyEqual(var1, var2, tol*(std::abs(var1)+std::abs(var2))));
}

bool
relativeFuzzyGreaterEqual(const Real & var1, const Real & var2, const Real & tol)
{
  return (absoluteFuzzyGreaterEqual(var1, var2, tol*(std::abs(var1)+std::abs(var2))));
}

bool
relativeFuzzyGreaterThan(const Real & var1, const Real & var2, const Real & tol)
{
  return (absoluteFuzzyGreaterThan(var1, var2, tol*(std::abs(var1)+std::abs(var2))));
}

bool
relativeFuzzyLessEqual(const Real & var1, const Real & var2, const Real & tol)
{
  return (absoluteFuzzyLessEqual(var1, var2, tol*(std::abs(var1)+std::abs(var2))));
}

bool
relativeFuzzyLessThan(const Real & var1, const Real & var2, const Real & tol)
{
  return (absoluteFuzzyLessThan(var1, var2, tol*(std::abs(var1)+std::abs(var2))));
}

void
MaterialPropertyStorageDump(const HashMap<const libMesh::Elem *, HashMap<unsigned int, MaterialProperties> > & props)
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

void
indentMessage(const std::string & prefix, std::string & message, const char* color/*= COLOR_CYAN*/)
{
  /**
   * First we need to see if the message we need to indent (with color) also contains color codes that span lines.
   * The code matches all of the XTERM constants (see XTermConstants.h). If it does, then we'll work on formatting
   * each colored multiline chunk one at a time with the right codes.
   */
  pcrecpp::RE re_color_piece("(\\33\\[3[0-7]m)([^\n]*\n.*\\33\\[3\\dm)", pcrecpp::DOTALL());
  std::string color_code, string_piece;
  std::string colored_message;
  pcrecpp::StringPiece input(message);

  bool initial_indent = false;
  while (re_color_piece.FindAndConsume(&input, &color_code, &string_piece))
  {
    // The colored prefix
    std::string indent = color + prefix + ": " + color_code;

    // Indent all the lines in this section
    const static pcrecpp::RE re("\n");
    re.GlobalReplace(std::string("\n") + indent, &string_piece);

    // Prepend indent string at the front of the message and assign to colored_message
    colored_message += indent + string_piece;
    initial_indent = true;
  }

  // Here we format the remainder of the message that doesn't have any multi-line color codes.

  /**
   * If we already colored part of the message we need to pick up where we left off. We'll
   * do that by using the StringPiece objects buffer. It will have already thrown away part
   * of string that it has reformatted.
   */
  input.CopyToString(&string_piece);

  // No multiline coloring, so we termintate each prefix with default coloring
  std::string indent = color + prefix + ": " + COLOR_DEFAULT;

  // Indent all the lines until the final newline is encountered
  const static pcrecpp::RE re("\n(?=.*\n)");
  re.GlobalReplace(std::string("\n") + indent, &string_piece);

  // Prepend indent string at the front of the message if it hasn't already been done
  if (!initial_indent)
    message = colored_message + indent + string_piece;
  else
    message = colored_message + string_piece;
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

  // Loop through all of the newest files according the number in the file name
  int max_file_num = -1;
  std::string max_base;
  pcrecpp::RE re_base_and_file_num("(.*?(\\d+))\\..*"); // Will pull out the full base and the file number simultaneously

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

} // MooseUtils namespace
