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

#include "ExodusFormatter.h"
#include "Parser.h"
#include "MooseApp.h"
// libMesh
#include "libmesh/exodusII.h"
//
#include <sstream>
#include <vector>


ExodusFormatter::ExodusFormatter() :
    InputFileFormatter(false)
{
}

void
ExodusFormatter::printInputFile(ActionWarehouse & wh)
{
  _ss << "####################\n"
      << "# Created by MOOSE #\n"
      << "####################\n";

  // Grab the command line arguments first
  _ss << "### Command Line Arguments ###\n";
  if(wh.mooseApp().commandLine())
    wh.mooseApp().commandLine()->print("", _ss, 1);

  _ss << "### Version Info ###\n"
      << wh.mooseApp().getSysInfo() << "\n";

  _ss << "### Input File ###" << std::endl;
  wh.printInputFile(_ss);
}

void
ExodusFormatter::format()
{
  std::string s;
  _input_file_record.clear();

  while (std::getline(_ss, s))
  {
    // MAX_LINE_LENGTH is from ExodusII
    if ( s.length() > MAX_LINE_LENGTH )
    {
      const std::string continuation("...");
      const size_t cont_len(continuation.length());
      size_t num_lines = s.length() / (MAX_LINE_LENGTH - cont_len) + 1;
      std::string split_line;
      for (size_t j(0), l_begin(0); j < num_lines; ++j, l_begin+=MAX_LINE_LENGTH-cont_len)
      {
        size_t l_len = MAX_LINE_LENGTH-cont_len;
        if (s.length() < l_begin + l_len )
        {
          l_len = s.length() - l_begin;
        }
        split_line = s.substr( l_begin, l_len );
        if ( l_begin + l_len != s.length())
        {
          split_line += continuation;
        }
        _input_file_record.push_back(split_line);
      }
    }
    else
    {
      _input_file_record.push_back(s);
    }
  }
}
