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
#include "MooseError.h"

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <istream>
#include <iterator>

// External includes
#include "pcrecpp.h"

namespace MooseUtils
{

void
tokenize(const std::string &str, std::vector<std::string> &elements, unsigned int min_len, const std::string &delims)
{
  elements.clear();

  std::string::size_type last_pos = str.find_first_not_of(delims, 0);
  std::string::size_type pos = str.find_first_of(delims, std::min(last_pos + min_len, str.size()));

  while (last_pos != std::string::npos)
  {
    elements.push_back(str.substr(last_pos, pos - last_pos));
    // skip delims between tokens
    last_pos = str.find_first_not_of(delims, pos);
    if (last_pos == std::string::npos) break;
    pos = str.find_first_of(delims, std::min(last_pos + min_len, str.size()));
  }
}

void
escape(std::string &str)
{
  std::map<char, std::string> escapes;
  escapes['\a'] = "\\a";
  escapes['\b'] = "\\b";
  escapes['\f'] = "\\f";
  escapes['\n'] = "\\n";
  escapes['\t'] = "\\t";
  escapes['\v'] = "\\v";
  escapes['\r'] = "\\r";

  for (std::map<char, std::string>::iterator it = escapes.begin(); it != escapes.end(); ++it)
    for (size_t pos=0; (pos=str.find(it->first, pos)) != std::string::npos; pos+=it->second.size())
      str.replace(pos, 1, it->second);
}


std::string
trim(std::string str, const std::string &white_space)
{
  std::string r = str.erase(str.find_last_not_of(white_space)+1);
  return r.erase(0,r.find_first_not_of(white_space));
}

bool pathContains(const std::string &expression,
                          const std::string &string_to_find,
                          const std::string &delims)
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
      mooseError((std::string("Unable to open file \"") + filename
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
      mooseError((std::string("Unable to open file \"") + filename
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
  if (full_file[full_file.size()-1] == '/')
    mooseError("Invalid full file name: " << full_file);

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

bool
absoluteFuzzyEqual(const Real & var1, const Real & var2, const Real & tol)
{
  return (std::abs(var1 - var2) < tol);
}

bool
absoluteFuzzyGreaterEqual(const Real & var1, const Real & var2, const Real & tol)
{
  return (var1 > (var2 - tol));
}

bool
absoluteFuzzyGreaterThan(const Real & var1, const Real & var2, const Real & tol)
{
  return (var1 > (var2 + tol));
}

bool
absoluteFuzzyLessEqual(const Real & var1, const Real & var2, const Real & tol)
{
  return (var1 < (var2 + tol));
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

} // MooseUtils namespace
