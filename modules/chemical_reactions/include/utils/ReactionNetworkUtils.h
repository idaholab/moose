//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include <string>
#include <optional>

namespace ReactionNetworkUtils
{
// Helper structs for parsing reaction network
struct Term
{
  double coefficient;
  std::string species;
  // State is between parenthesis
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

  /// Get all species involved in the reaction
  std::vector<VariableName> getSpecies() const;

  /// Get the stoeichiometric coefficients for each species in the reaction
  std::vector<Real> getStoichiometricCoefficients() const;

  /// Get all reactant species involved in the reaction (on LHS)
  std::vector<VariableName> getReactantSpecies() const;

  /// Get all product species involved in the reaction (on RHS)
  std::vector<std::string> getProductSpecies() const;

  /// Whether the reaction has the metadata with the given type
  template <typename T>
  bool hasMetaData(const std::string & key) const;

  /// Whether the reaction has the metadata
  bool hasMetaData(const std::string & key) const;

  /// Get the metadata from the reaction
  const std::string & getMetaData(const std::string & key) const;
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
