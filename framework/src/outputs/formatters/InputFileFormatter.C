//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InputFileFormatter.h"
#include "MooseUtils.h"
#include "InputParameters.h"

#include <sstream>
#include <vector>
#include <iomanip>

InputFileFormatter::InputFileFormatter(bool dump_mode) : SyntaxTree(), _dump_mode(dump_mode) {}

std::string
InputFileFormatter::printBlockOpen(const std::string & name,
                                   short depth,
                                   const std::string & /*doc*/)
{
  std::string indent(depth * 2, ' ');
  std::string opening_string;

  if (depth)
    opening_string = "./";

  return std::string("\n") + indent + "[" + opening_string + name + "]\n";
}

std::string
InputFileFormatter::printBlockClose(const std::string & /*name*/, short depth) const
{
  std::string indent(depth * 2, ' ');
  std::string closing_string;

  if (depth)
    closing_string = "../";

  return std::string("") + indent + "[" + closing_string + "]\n";
}

std::string
InputFileFormatter::printParams(const std::string & /*prefix*/,
                                const std::string & fully_qualified_name,
                                InputParameters & params,
                                short depth,
                                const std::string & search_string,
                                bool & found)
{
  std::stringstream oss;

  std::string quotes = "";
  std::string spacing = "";
  std::string forward = "";
  std::string backdots = "";
  int offset = 30;
  for (int i = 0; i < depth; ++i)
  {
    spacing += "  ";
    forward = ".";
    offset -= 2;
  }

  for (const auto & iter : params)
  {
    // We only want non-private params and params that we haven't already seen
    if (params.isPrivate(iter.first) || haveSeenIt(fully_qualified_name, iter.first))
      continue;

    std::string value = "INVALID";
    if (params.isParamValid(iter.first))
    {
      // Print the parameter's value to a stringstream.
      std::ostringstream toss;
      iter.second->print(toss);
      value = MooseUtils::trim(toss.str());
    }
    else if (params.hasDefaultCoupledValue(iter.first))
    {
      std::ostringstream toss;
      toss << params.defaultCoupledValue(iter.first);
      value = toss.str();
    }

    // See if we match the search string
    if (MooseUtils::wildCardMatch(iter.first, search_string) ||
        MooseUtils::wildCardMatch(value, search_string))
    {
      // Don't print active if it is the default all, that means it's not in the input file - unless
      // of course we are in dump mode
      if (!_dump_mode && iter.first == "active")
      {
        if (params.have_parameter<std::vector<std::string>>(iter.first))
        {
          const auto & active = params.get<std::vector<std::string>>(iter.first);
          if (active.size() == 1 && active[0] == "__all__")
            continue;
        }
      }

      // Mark it as "seen"
      seenIt(fully_qualified_name, iter.first);

      // Don't print type if it is blank
      if (iter.first == "type")
      {
        if (params.have_parameter<std::string>(iter.first))
        {
          const auto & active = params.get<std::string>(iter.first);
          if (active == "")
            continue;
        }
      }

      found = true;
      oss << spacing << "  " << std::left << std::setw(offset) << iter.first << " = ";
      // std::setw() takes an int
      int l_offset = 30;

      if (!_dump_mode || value != "INVALID")
      {
        // If the value has spaces, surround it with quotes, otherwise no quotes
        if (value.find(' ') != std::string::npos)
        {
          quotes = "'";
          l_offset -= 2;
        }
        else
          quotes = "";

        if (value.size() == 0)
          value = "(no_default)";
        oss << quotes << value << quotes;
        l_offset -= value.size();
      }
      else if (_dump_mode && params.isParamRequired(iter.first))
      {
        oss << "(required)";
        l_offset -= 10;
      }

      // Documentation string
      if (_dump_mode)
      {
        std::vector<std::string> elements;
        std::string doc = params.getDocString(iter.first);
        if (MooseUtils::trim(doc) != "")
        {
          MooseUtils::tokenize(doc, elements, 68, " \t");

          for (auto & element : elements)
            MooseUtils::escape(element);

          oss << std::right << std::setw(l_offset) << "# " << elements[0];
          for (unsigned int i = 1; i < elements.size(); ++i)
            oss << " ...\n"
                << "  " << std::setw(63) << "# " << elements[i];
        }
        const std::string group = params.getGroupName(iter.first);
        if (!group.empty())
        {
          if (MooseUtils::trim(doc) != "")
            oss << " ...\n"
                << "  " << std::setw(70) << "# Group: " << group;
          else
            oss << std::right << std::setw(l_offset) << "# Group: " << group;
        }
      }
      oss << std::endl;
    }
  }

  return oss.str();
}
