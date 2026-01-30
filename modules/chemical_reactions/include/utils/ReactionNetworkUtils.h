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

  // We don't compare the coefficient on purpose, to lump terms with different coefficients
  // together
  // NOTE: if you have a need to differentiate terms by terms by coefficients, you'll have
  // to create a custom comparator
  friend bool operator<(const Term & a, const Term & b) noexcept
  {
    if (a.species != b.species)
      return a.species < b.species;
    if (a.state != b.state)
      return a.state < b.state;
    return a.charge < b.charge;
  }
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
  /// Get all unique species involved in the reaction
  /// Note: a species will only appear once even if both a reactant and a product
  std::vector<VariableName> getUniqueSpecies() const;

  /// Get the stoeichiometric coefficients for each species in the reaction
  /// The sign used matches the sign in the reaction equation.
  std::vector<Real> getStoichiometricCoefficients() const;
  /// Get the stoeichiometric coefficients for each unique species in the reaction
  /// Note: if a species is both a reactant and a product,
  /// the coefficient will be the difference between its product and reactor coefficients
  /// Note: this means all coefficients for reactants that are not products are negative,
  /// unlike in getStoichiometricCoefficients()
  std::map<VariableName, Real> getUniqueStoichiometricCoefficients() const;

  /// Get all reactant species involved in the reaction (on LHS)
  std::vector<VariableName> getReactantSpecies() const;
  /// Get all unique reactant species involved in the reaction (on LHS)
  std::vector<VariableName> getUniqueReactantSpecies() const;

  /// Get all product species involved in the reaction (on RHS)
  std::vector<std::string> getProductSpecies() const;
  /// Get all unique product species involved in the reaction (on RHS)
  std::vector<std::string> getUniqueProductSpecies() const;

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
