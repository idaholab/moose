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

namespace MooseUtils
{

void
tokenize(const std::string &str, std::vector<std::string> &elements, unsigned int min_len, const std::string &delims)
{
  elements.clear();

  std::string::size_type last_pos = str.find_first_not_of(delims, 0);
  std::string::size_type pos = str.find_first_of(delims, std::min(last_pos + min_len, str.size()));

  while (pos != std::string::npos || last_pos != std::string::npos)
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

void
checkFileReadable(const std::string & filename, bool check_line_endings)
{
  std::ifstream in(filename.c_str(), std::ifstream::in);
  if (in.fail())
    mooseError((std::string("Unable to open file \"") + filename
                + std::string("\". Check to make sure that it exists and that you have read permission.")).c_str());

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
}

void
checkFileWriteable(const std::string & filename)
{
  std::ofstream out(filename.c_str(), std::ofstream::out);
  if (out.fail())
    mooseError((std::string("Unable to open file \"") + filename
                + std::string("\". Check to make sure that it exists and that you have write permission.")).c_str());

  out.close();
}

void
parallelBarrierNotify()
{
  processor_id_type slave_processor_id;

  if (libMesh::processor_id() == 0)
  {
    // The master process is already through, so report it
    Moose::out << "Jobs complete: 1/" << libMesh::n_processors() << "\r" << std::flush;
    for (unsigned int i=2; i<=libMesh::n_processors(); ++i)
    {
      Parallel::receive(MPI_ANY_SOURCE, slave_processor_id);
      Moose::out << "Jobs complete: " << i << "/" << libMesh::n_processors() << (i == libMesh::n_processors() ? "\n" : "\r") << std::flush;
    }
  }
  else
  {
    slave_processor_id = libMesh::processor_id();
    Parallel::send(0, slave_processor_id);
  }

  Parallel::barrier();
}

bool
hasExtension(const std::string & filename, std::string ext)
{
  if (filename.substr(filename.find_last_of(".") + 1) == ext)
    return true;
  else
    return false;
}

} // MooseUtils namespace
