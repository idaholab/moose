//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusInputParser.h"

namespace AbaqusInputParser
{

bool
AbaqusUELMesh::readDataLine(std::string & s)
{
  s.clear();

  while (true)
  {
    if (_in->peek() == '*' || _in->peek() == EOF)
    {
      if (s.empty())
        return false;
      mooseError("Incomplete data line.");
    }

    // read line
    auto l = readLine();
    strip(l);
    s += l;

    // check if line continuation is needed
    if (s.back() != ',')
      return true;
  }
}

Root::Root(std::istream file) : Block(*this), _file(file)
{
  // register syntax
  registerBlock<Part>("part");
  registerBlock<Assembly>("assembly");
  registerBlock<Step>("step");
  registerOption<Boundary>("boundary");
  registerOption<Initial Conditions>("initial conditions");

  // load and preprocess entire file
  loadFile();
}

Root::loadFile()
{
  _current_line = 0;

  std::string s, ss;
  while (true)
  {
    if (_file.eof())
      break;

    // read line
    std::getline(_file, s);
    ss = s;

    // continuation lines
    while (ss.back() != ',')
    {
      std::getline(_file, s);

      // error, expected a continuation line but got EOF
      if (_file.eof())
        throw EndOfAbaqusInput();

      ss += s;
    }

    // skip empty lines and comments (todo: remove spaces)
    if (ss.length() < 2 || ss.substr(0, 2) == "**")
      continue;

    std::vector<std::string> items;
    MooseUtils::tokenize(ss, items, 1, ",");
    _lines.push_back(items);
  }
}

Assembly::Assembly(const Root &) : Block(root)
{
  registerBlock<Instrance>("instance");
  registerOption<Nset>("nset");
  registerOpt<Elset>("elset");
}

} // namespace AbaqusInputParser
