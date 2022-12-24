//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "YAMLFormatter.h"

// MOOSE includes
#include "MooseEnum.h"
#include "MultiMooseEnum.h"
#include "Parser.h"

#include "libmesh/vector_value.h"
#include "libmesh/point.h"

// C++ includes
#include <sstream>
#include <vector>

YAMLFormatter::YAMLFormatter(bool dump_mode) : SyntaxTree(true), _dump_mode(dump_mode) {}

std::string
YAMLFormatter::preamble() const
{
  // important: start and end yaml data delimiters used by python
  return "**START YAML DATA**\n";
}

std::string
YAMLFormatter::postscript() const
{
  return "**END YAML DATA**\n";
}

std::string
YAMLFormatter::printParams(const std::string & prefix,
                           const std::string & /*fully_qualified_name*/,
                           InputParameters & params,
                           short depth,
                           const std::string & search_string,
                           bool & found)
{
  std::ostringstream oss;
  std::string indent(depth * 2, ' ');

  for (auto & iter : params)
  {
    std::string name = iter.first;
    // First make sure we want to see this parameter, also block active and type
    if (params.isPrivate(iter.first) || name == "active" ||
        (search_string != "" && search_string != iter.first) || haveSeenIt(prefix, iter.first))
      continue;

    found = true;

    // Mark it as "seen"
    seenIt(prefix, iter.first);

    // Block params may be required and will have a doc string
    std::string required = params.isParamRequired(iter.first) ? "Yes" : "No";

    oss << indent << "  - name: " << name << "\n";
    oss << indent << "    required: " << required << "\n";
    oss << indent << "    default: !!str ";

    // Only output default if it has one
    if (params.isParamValid(iter.first))
    {
      // prints the value, which is the default value when dumping the tree
      // because it hasn't been changed

      // Output stream, performing special operations for writing objects such as Points and
      // RealVectorValues
      std::ostringstream toss;
      buildOutputString(toss, iter);

      // remove additional '\n' possibly generated in output (breaks YAML parsing)
      std::string tmp_str = toss.str();
      for (auto & ch : tmp_str)
        if (ch == '\n')
          ch = ' ';
      if (tmp_str == ",")
        oss << "\"" << tmp_str << "\"";
      else
        oss << tmp_str;
    }
    else if (params.hasDefaultCoupledValue(iter.first))
      oss << params.defaultCoupledValue(iter.first);

    std::string doc = params.getDocString(iter.first);
    MooseUtils::escape(doc);
    // Print the type
    oss << "\n"
        << indent << "    cpp_type: " << params.type(iter.first) << "\n"
        << indent << "    group_name: ";
    std::string group_name = params.getGroupName(iter.first);
    if (!group_name.empty())
      oss << "'" << group_name << "'";
    oss << "\n";

    if (params.have_parameter<MooseEnum>(name))
      addEnumOptionsAndDocs(oss, params.get<MooseEnum>(name), indent);
    if (params.have_parameter<MultiMooseEnum>(name))
      addEnumOptionsAndDocs(oss, params.get<MultiMooseEnum>(name), indent);
    if (params.have_parameter<ExecFlagEnum>(name))
      addEnumOptionsAndDocs(oss, params.get<ExecFlagEnum>(name), indent);
    if (params.have_parameter<std::vector<MooseEnum>>(name))
      addEnumOptionsAndDocs(oss, params.get<std::vector<MooseEnum>>(name)[0], indent);

    oss << indent << "    description: |\n      " << indent << doc << std::endl;
  }

  return oss.str();
}

template <typename T>
void
YAMLFormatter::addEnumOptionsAndDocs(std::ostringstream & oss,
                                     T & param,
                                     const std::string & indent)
{
  oss << indent << "    options: " << param.getRawNames() << '\n';
  const auto & docs = param.getItemDocumentation();
  if (!docs.empty())
  {
    oss << indent << "    option_docs:\n";
    for (const auto & doc : docs)
    {
      oss << indent << "    - name: " << doc.first.name() << "\n";
      oss << indent << "      description: |\n";
      oss << indent << "        " << doc.second << "\n";
    }
  }
}

std::string
YAMLFormatter::preTraverse(short depth) const
{
  std::string indent(depth * 2, ' ');

  return indent + "  subblocks:\n";
}

std::string
YAMLFormatter::printBlockOpen(const std::string & name, short depth, const std::string & doc)
{
  std::ostringstream oss;
  std::string indent(depth * 2, ' ');

  std::string docEscaped = doc;
  MooseUtils::escape(docEscaped);

  oss << indent << "- name: " << name << "\n";
  oss << indent << "  description: |\n" << indent << "    " << docEscaped << "\n";
  oss << indent << "  parameters:\n";

  return oss.str();
}

std::string
YAMLFormatter::printBlockClose(const std::string & /*name*/, short /*depth*/) const
{
  return std::string();
}

void
YAMLFormatter::buildOutputString(
    std::ostringstream & output,
    const std::iterator_traits<InputParameters::iterator>::value_type & p)
{
  libMesh::Parameters::Value * val = MooseUtils::get(p.second);

  // Account for Point
  InputParameters::Parameter<Point> * ptr0 = dynamic_cast<InputParameters::Parameter<Point> *>(val);

  // Account for RealVectorValues
  InputParameters::Parameter<RealVectorValue> * ptr1 =
      dynamic_cast<InputParameters::Parameter<RealVectorValue> *>(val);

  // Output the Point components
  if (ptr0)
    output << ptr0->get().operator()(0) << " " << ptr0->get().operator()(1) << " "
           << ptr0->get().operator()(2);

  // Output the RealVectorValue components
  else if (ptr1)
    output << ptr1->get().operator()(0) << " " << ptr1->get().operator()(1) << " "
           << ptr1->get().operator()(2);

  // General case, call the print operator
  else
    p.second->print(output);
}
