//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <string>

namespace ReactionNetworkUtils
{
// Helper structs for parsing reaction network
struct Term
{
  double coefficient;
  std::string species;
  std::optional<std::string> state;
  std::optional<std::string> charge;
};

using TermList = std::vector<Term>;
using Metadata = std::map<std::string, std::string>;

struct Reaction
{
  TermList reactants;
  TermList products;
  Metadata metadata;
};

/// @brief  Parses the reaction network from a string form to a vector a Reaction
/// @param reaction_network_string
/// @return
std::vector<Reaction> parseReactionNetwork(const std::string & reaction_network_string,
                                           bool output_to_cout);
}

namespace Moose
{
std::string stringify(const ReactionNetworkUtils::Reaction & t);
}
