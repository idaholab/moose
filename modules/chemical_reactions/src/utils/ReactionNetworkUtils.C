//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReactionNetworkUtils.h"
#include "MooseTypes.h"
#include "MooseUtils.h"
#include "libmesh/utility.h"
#include "peglib.h"

namespace ReactionNetworkUtils
{

std::vector<Reaction>
parseReactionNetwork(const std::string & reaction_network_string, bool output_to_cout)
{
  using namespace peg;
  parser parser(R"(
    ReactionNetwork <- (_ Reaction _ Newline?)* _
    Reaction        <- _ TermList _ '->' _ TermList _ Metadata?
    TermList        <- Term (_ '+' _ Term)*
    Term            <- Coefficient? Species
    Coefficient     <- Float
    Float           <- [0-9]+ ('.' [0-9]+)?
    Species         <- BaseSpecies State? Charge?
    BaseSpecies     <- [A-Z][a-zA-Z0-9_]*
    State           <- '(' [a-z]{1,3} ')'
    Charge          <- ('^'? [0-9]* [+-])
    Metadata        <- '[' _ Pair (_ ',' _ Pair)* _ ']'
    Pair            <- Key _ '=' _ Value
    Key             <- [A-Za-z_][A-Za-z0-9_]*
    Value           <- [A-Za-z0-9_.-]+
    ~_              <- [ \t]*
  )");

  // Rules for how to parse each type
  parser["ReactionNetwork"] = [](const SemanticValues & vs)
  {
    std::vector<Reaction> reactions;
    for (const auto & v : vs)
    {
      if (v.type() == typeid(Reaction))
      {
        reactions.push_back(std::any_cast<Reaction>(v));
      }
    }
    return reactions;
  };

  parser["Float"] = [](const SemanticValues & vs) { return std::stod(std::string(vs.token())); };

  parser["Coefficient"] = [](const SemanticValues & vs) { return std::any_cast<double>(vs[0]); };

  parser["BaseSpecies"] = [](const SemanticValues & vs) { return vs.token(); };

  parser["State"] = [](const SemanticValues & vs)
  {
    std::string s = std::string(vs.token());
    return s.substr(1, s.length() - 2); // remove parentheses
  };

  parser["Charge"] = [](const SemanticValues & vs) { return vs.token(); };

  parser["Species"] = [](const SemanticValues & vs)
  {
    std::string base = std::any_cast<std::string>(vs[0]);
    std::optional<std::string> state;
    std::optional<std::string> charge;

    if (vs.size() >= 2)
    {
      for (size_t i = 1; i < vs.size(); ++i)
      {
        if (vs[i].type() == typeid(std::string))
        {
          std::string val = std::any_cast<std::string>(vs[i]);
          if (val.front() == '(' && val.back() == ')')
            state = val.substr(1, val.length() - 2);
          else
            charge = val;
        }
      }
    }

    return Term{1.0, base, state, charge}; // default coefficient = 1.0
  };

  parser["Term"] = [](const SemanticValues & vs)
  {
    if (vs.size() == 2)
    {
      double coeff = std::any_cast<double>(vs[0]);
      Term term = std::any_cast<Term>(vs[1]);
      term.coefficient = coeff;
      return term;
    }
    else
    {
      return std::any_cast<Term>(vs[0]);
    }
  };

  parser["TermList"] = [](const SemanticValues & vs)
  {
    TermList terms;
    for (const auto & v : vs)
    {
      if (v.type() == typeid(Term))
        terms.push_back(std::any_cast<Term>(v));
      else if (v.type() == typeid(std::vector<Term>))
      {
        const auto & more = std::any_cast<std::vector<Term>>(v);
        terms.insert(terms.end(), more.begin(), more.end());
      }
    }
    return terms;
  };

  parser["Key"] = [](const SemanticValues & vs) { return vs.token(); };

  parser["Value"] = [](const SemanticValues & vs) { return vs.token(); };

  parser["Pair"] = [](const SemanticValues & vs)
  { return std::make_pair(std::any_cast<std::string>(vs[0]), std::any_cast<std::string>(vs[1])); };

  parser["Metadata"] = [](const SemanticValues & vs)
  {
    Metadata meta;
    for (const auto & v : vs)
    {
      if (v.type() == typeid(std::pair<std::string, std::string>))
      {
        const auto & [k, val] = std::any_cast<std::pair<std::string, std::string>>(v);
        meta[k] = val;
      }
      else if (v.type() == typeid(std::vector<std::pair<std::string, std::string>>))
      {
        const auto & pairs = std::any_cast<std::vector<std::pair<std::string, std::string>>>(v);
        for (const auto & [k, val] : pairs)
          meta[k] = val;
      }
    }
    return meta;
  };

  parser["Reaction"] = [](const SemanticValues & vs)
  {
    Reaction r;
    r.reactants = std::any_cast<TermList>(vs[0]);
    r.products = std::any_cast<TermList>(vs[1]);
    if (vs.size() == 3)
      r.metadata = std::any_cast<Metadata>(vs[2]);
    return r;
  };

  parser.enable_packrat_parsing();

  std::vector<Reaction> reactions;
  if (!parser.parse(reaction_network_string, reactions))
  {
    throw std::runtime_error("Failed to parse reaction.");
  }

  // Output the reaction network
  if (output_to_cout)
  {
    for (size_t i = 0; i < reactions.size(); ++i)
    {
      const auto & r = reactions[i];
      std::cout << "Reaction " << i + 1 << ":\n";
      std::cout << Moose::stringify(r) << std::endl;
    }
  }

  return reactions;
}

std::vector<VariableName>
Reaction::getSpecies() const
{
  std::vector<VariableName> species;
  for (const auto & term : reactants)
    species.push_back(term.species);
  for (const auto & term : products)
    species.push_back(term.species);
  return species;
}

std::vector<Real>
Reaction::getStoichiometricCoefficients() const
{
  std::vector<Real> coeffs;
  for (const auto & term : reactants)
    coeffs.push_back(term.coefficient);
  for (const auto & term : products)
    coeffs.push_back(term.coefficient);
  return coeffs;
}

std::vector<std::string>
Reaction::getProductSpecies() const
{
  std::vector<std::string> species;
  for (const auto & term : products)
    species.push_back(term.species);
  return species;
}

template <>
bool
Reaction::hasMetaData<Real>(const std::string & key) const
{
  return metadata.count(key) && MooseUtils::isDigits(libmesh_map_find(metadata, key));
}

template <typename T>
bool
Reaction::hasMetaData(const std::string & /*key*/) const
{
  ::mooseError("Has metadata not implemented for type " + MooseUtils::prettyCppType<T>());
}

bool
Reaction::hasMetaData(const std::string & key) const
{
  return metadata.count(key);
}

const std::string &
Reaction::getMetaData(const std::string & key) const
{
  return libmesh_map_find(metadata, key);
}

// namespace ReactionNetworkUtils
}

namespace Moose
{
std::string
stringify(const ReactionNetworkUtils::Reaction & r)
{
  std::string str_out = "  Reactants:\n";
  for (const auto & t : r.reactants)
  {
    str_out += "    " + std::to_string(t.coefficient) + " " + t.species;
    if (t.charge)
      str_out += t.charge.value();
    if (t.state)
      str_out += " (" + t.state.value() + ")";
    str_out += "\n";
  }
  str_out += "  Products:\n";
  for (const auto & t : r.products)
  {
    str_out += "    " + std::to_string(t.coefficient) + " " + t.species;
    if (t.charge)
      str_out += t.charge.value();
    if (t.state)
      str_out += " (" + t.state.value() + ")";
    str_out += "\n";
  }
  if (!r.metadata.empty())
  {
    str_out += "  Metadata:\n";
    for (const auto & [k, v] : r.metadata)
      str_out += "    " + k + " = " + v + "\n";
  }
  return str_out;
}
}
