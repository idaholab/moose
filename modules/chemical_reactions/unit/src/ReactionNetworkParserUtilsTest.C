//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "ReactionNetworkUtils.h"

#include <cmath>

TEST(ReactionNetworkParserUtilsTest, simple)
{
  // Simple
  auto reaction = ReactionNetworkUtils::parseReactionNetwork("A -> B", false)[0];
  EXPECT_EQ(reaction.getSpecies()[0], "A");
  EXPECT_EQ(reaction.getSpecies()[1], "B");
  EXPECT_EQ(reaction.getReactantSpecies()[0], "A");
  EXPECT_EQ(reaction.getProductSpecies()[0], "B");
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[0], 1);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[1], 1);
  EXPECT_EQ(reaction.hasMetaData("this"), false);
  try
  {
    reaction.getMetaData("this");
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos =
        std::string(err.what()).find("MetaData item 'this' was not found in reaction");
    ASSERT_TRUE(pos != std::string::npos);
  }

  // With charge
  reaction = ReactionNetworkUtils::parseReactionNetwork("A+ -> B-", false)[0];
  EXPECT_EQ(reaction.getSpecies()[0], "A+");
  EXPECT_EQ(reaction.getSpecies()[1], "B-");
  EXPECT_EQ(reaction.getReactantSpecies()[0], "A+");
  EXPECT_EQ(reaction.getProductSpecies()[0], "B-");
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[0], 1);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[1], 1);
  EXPECT_EQ(reaction.hasMetaData("this"), false);

  // With state
  reaction = ReactionNetworkUtils::parseReactionNetwork("A(aq)+ -> B(gas)-", false)[0];
  EXPECT_EQ(reaction.getSpecies()[0], "A(aq)+");
  EXPECT_EQ(reaction.getSpecies()[1], "B(gas)-");
  EXPECT_EQ(reaction.getReactantSpecies()[0], "A(aq)+");
  EXPECT_EQ(reaction.getProductSpecies()[0], "B(gas)-");
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[0], 1);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[1], 1);
  EXPECT_EQ(reaction.hasMetaData("this"), false);
}

TEST(ReactionNetworkParserUtilsTest, minus_sign)
{
  // Simple
  auto reaction = ReactionNetworkUtils::parseReactionNetwork("-A -> -B", false)[0];
  EXPECT_EQ(reaction.getSpecies()[0], "A");
  EXPECT_EQ(reaction.getSpecies()[1], "B");
  EXPECT_EQ(reaction.getReactantSpecies()[0], "A");
  EXPECT_EQ(reaction.getProductSpecies()[0], "B");
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[0], -1);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[1], -1);
  EXPECT_EQ(reaction.hasMetaData("this"), false);

  // With charge
  reaction = ReactionNetworkUtils::parseReactionNetwork("-A+ -> -B-", false)[0];
  EXPECT_EQ(reaction.getSpecies()[0], "A+");
  EXPECT_EQ(reaction.getSpecies()[1], "B-");
  EXPECT_EQ(reaction.getReactantSpecies()[0], "A+");
  EXPECT_EQ(reaction.getProductSpecies()[0], "B-");
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[0], -1);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[1], -1);
  EXPECT_EQ(reaction.hasMetaData("this"), false);

  // With state
  reaction = ReactionNetworkUtils::parseReactionNetwork("-A(aq)+ -> -B(gas)-", false)[0];
  EXPECT_EQ(reaction.getSpecies()[0], "A(aq)+");
  EXPECT_EQ(reaction.getSpecies()[1], "B(gas)-");
  EXPECT_EQ(reaction.getReactantSpecies()[0], "A(aq)+");
  EXPECT_EQ(reaction.getProductSpecies()[0], "B(gas)-");
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[0], -1);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[1], -1);
  EXPECT_EQ(reaction.hasMetaData("this"), false);
}

TEST(ReactionNetworkParserUtilsTest, two_terms)
{
  // Simple
  auto reaction = ReactionNetworkUtils::parseReactionNetwork("-A + C -> -B", false)[0];
  EXPECT_EQ(reaction.getSpecies()[0], "A");
  EXPECT_EQ(reaction.getSpecies()[1], "C");
  EXPECT_EQ(reaction.getSpecies()[2], "B");
  EXPECT_EQ(reaction.getReactantSpecies()[0], "A");
  EXPECT_EQ(reaction.getReactantSpecies()[1], "C");
  EXPECT_EQ(reaction.getProductSpecies()[0], "B");
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[0], -1);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[1], 1);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[2], -1);
  EXPECT_EQ(reaction.hasMetaData("this"), false);

  // Coefficient
  reaction = ReactionNetworkUtils::parseReactionNetwork("-10A + 3.4C -> -2.1111111B", false)[0];
  EXPECT_EQ(reaction.getSpecies()[0], "A");
  EXPECT_EQ(reaction.getSpecies()[1], "C");
  EXPECT_EQ(reaction.getSpecies()[2], "B");
  EXPECT_EQ(reaction.getReactantSpecies()[0], "A");
  EXPECT_EQ(reaction.getReactantSpecies()[1], "C");
  EXPECT_EQ(reaction.getProductSpecies()[0], "B");
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[0], -10);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[1], 3.4);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[2], -2.1111111);
  EXPECT_EQ(reaction.hasMetaData("this"), false);

  // Two products
  reaction =
      ReactionNetworkUtils::parseReactionNetwork("-10A + 3.4C -> -2.1111111B + 24D", false)[0];
  EXPECT_EQ(reaction.getSpecies()[0], "A");
  EXPECT_EQ(reaction.getSpecies()[1], "C");
  EXPECT_EQ(reaction.getSpecies()[2], "B");
  EXPECT_EQ(reaction.getSpecies()[3], "D");
  EXPECT_EQ(reaction.getReactantSpecies()[0], "A");
  EXPECT_EQ(reaction.getReactantSpecies()[1], "C");
  EXPECT_EQ(reaction.getProductSpecies()[0], "B");
  EXPECT_EQ(reaction.getProductSpecies()[1], "D");
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[0], -10);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[1], 3.4);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[2], -2.1111111);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[3], 24);
  EXPECT_EQ(reaction.hasMetaData("this"), false);
}

TEST(ReactionNetworkParserUtilsTest, multiple_reactions)
{
  // Simple
  auto reactions =
      ReactionNetworkUtils::parseReactionNetwork("-A + C -> -B\nD(aq) + E+ -> F - 1.1G", false);
  auto reaction = reactions[0];
  EXPECT_EQ(reaction.getSpecies()[0], "A");
  EXPECT_EQ(reaction.getSpecies()[1], "C");
  EXPECT_EQ(reaction.getSpecies()[2], "B");
  EXPECT_EQ(reaction.getReactantSpecies()[0], "A");
  EXPECT_EQ(reaction.getReactantSpecies()[1], "C");
  EXPECT_EQ(reaction.getProductSpecies()[0], "B");
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[0], -1);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[1], 1);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[2], -1);
  EXPECT_EQ(reaction.hasMetaData("this"), false);
  auto reaction2 = reactions[1];
  EXPECT_EQ(reaction2.getSpecies()[0], "D(aq)");
  EXPECT_EQ(reaction2.getSpecies()[1], "E+");
  EXPECT_EQ(reaction2.getSpecies()[2], "F");
  EXPECT_EQ(reaction2.getSpecies()[3], "G");
  EXPECT_EQ(reaction2.getReactantSpecies()[0], "D(aq)");
  EXPECT_EQ(reaction2.getReactantSpecies()[1], "E+");
  EXPECT_EQ(reaction2.getProductSpecies()[0], "F");
  EXPECT_EQ(reaction2.getProductSpecies()[1], "G");
  EXPECT_EQ(reaction2.getStoichiometricCoefficients()[0], 1);
  EXPECT_EQ(reaction2.getStoichiometricCoefficients()[1], 1);
  EXPECT_EQ(reaction2.getStoichiometricCoefficients()[2], 1);
  EXPECT_EQ(reaction2.getStoichiometricCoefficients()[3], -1.1);
  EXPECT_EQ(reaction2.hasMetaData("this"), false);
}

TEST(ReactionNetworkParserUtilsTest, non_unique_species)
{
  // Simple
  auto reactions = ReactionNetworkUtils::parseReactionNetwork(
      "A - A + A -> -B\nD(aq) + E+ -> F + 2.1F - 1.1G", false);
  auto reaction = reactions[0];
  EXPECT_EQ(reaction.getSpecies()[0], "A");
  EXPECT_EQ(reaction.getSpecies()[1], "A");
  EXPECT_EQ(reaction.getSpecies()[2], "A");
  EXPECT_EQ(reaction.getSpecies()[3], "B");
  EXPECT_EQ(reaction.getUniqueSpecies()[0], "A");
  EXPECT_EQ(reaction.getUniqueSpecies()[1], "B");
  EXPECT_EQ(reaction.getReactantSpecies()[0], "A");
  EXPECT_EQ(reaction.getUniqueReactantSpecies()[0], "A");
  EXPECT_EQ(reaction.getUniqueReactantSpecies().size(), 1);
  EXPECT_EQ(reaction.getProductSpecies()[0], "B");
  EXPECT_EQ(reaction.getUniqueProductSpecies()[0], "B");
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[0], 1);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[1], -1);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[3], -1);
  EXPECT_EQ(reaction.getUniqueStoichiometricCoefficients()["A"], 1);
  EXPECT_EQ(reaction.getUniqueStoichiometricCoefficients()["B"], 1);
  EXPECT_EQ(reaction.hasMetaData("this"), false);
  auto reaction2 = reactions[1];
  EXPECT_EQ(reaction2.getSpecies()[0], "D(aq)");
  EXPECT_EQ(reaction2.getSpecies()[1], "E+");
  EXPECT_EQ(reaction2.getSpecies()[2], "F");
  EXPECT_EQ(reaction2.getSpecies()[3], "F");
  EXPECT_EQ(reaction2.getSpecies()[4], "G");
  EXPECT_EQ(reaction2.getUniqueSpecies()[0], "D(aq)");
  EXPECT_EQ(reaction2.getUniqueSpecies()[1], "E+");
  EXPECT_EQ(reaction2.getUniqueSpecies()[2], "F");
  EXPECT_EQ(reaction2.getUniqueSpecies()[3], "G");
  EXPECT_EQ(reaction2.getReactantSpecies()[0], "D(aq)");
  EXPECT_EQ(reaction2.getReactantSpecies()[1], "E+");
  EXPECT_EQ(reaction2.getProductSpecies()[0], "F");
  EXPECT_EQ(reaction2.getProductSpecies()[2], "G");
  EXPECT_EQ(reaction2.getUniqueProductSpecies()[0], "F");
  EXPECT_EQ(reaction2.getUniqueProductSpecies()[1], "G");
  EXPECT_EQ(reaction2.getStoichiometricCoefficients()[0], 1);
  EXPECT_EQ(reaction2.getStoichiometricCoefficients()[1], 1);
  EXPECT_EQ(reaction2.getStoichiometricCoefficients()[2], 1);
  EXPECT_EQ(reaction2.getStoichiometricCoefficients()[3], 2.1);
  EXPECT_EQ(reaction2.getStoichiometricCoefficients()[4], -1.1);
  EXPECT_EQ(reaction2.getUniqueStoichiometricCoefficients()["D(aq)"], 1);
  EXPECT_EQ(reaction2.getUniqueStoichiometricCoefficients()["E+"], 1);
  EXPECT_EQ(reaction2.getUniqueStoichiometricCoefficients()["F"], -3.1);
  EXPECT_EQ(reaction2.getUniqueStoichiometricCoefficients()["G"], 1.1);
  EXPECT_EQ(reaction2.hasMetaData("this"), false);
}

TEST(ReactionNetworkParserUtilsTest, metadata)
{
  // One double term
  auto reaction = ReactionNetworkUtils::parseReactionNetwork(
      "-10A + 3.4C -> -2.1111111B + 24D [k=2.1]", false)[0];
  EXPECT_EQ(reaction.getSpecies()[0], "A");
  EXPECT_EQ(reaction.getSpecies()[1], "C");
  EXPECT_EQ(reaction.getSpecies()[2], "B");
  EXPECT_EQ(reaction.getSpecies()[3], "D");
  EXPECT_EQ(reaction.getReactantSpecies()[0], "A");
  EXPECT_EQ(reaction.getReactantSpecies()[1], "C");
  EXPECT_EQ(reaction.getProductSpecies()[0], "B");
  EXPECT_EQ(reaction.getProductSpecies()[1], "D");
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[0], -10);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[1], 3.4);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[2], -2.1111111);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[3], 24);
  EXPECT_EQ(reaction.hasMetaData("this"), false);
  EXPECT_EQ(reaction.hasMetaData<Real>("k"), true);
  EXPECT_EQ(reaction.hasMetaData("k"), true);
  EXPECT_EQ(reaction.getMetaData("k"), "2.1");

  // Two double terms (one negative) and a space in there
  reaction = ReactionNetworkUtils::parseReactionNetwork(
      "-10A + 3.4C -> -2.1111111B + 24D [k=2.1, Q=-2]", false)[0];
  EXPECT_EQ(reaction.getSpecies()[0], "A");
  EXPECT_EQ(reaction.getSpecies()[1], "C");
  EXPECT_EQ(reaction.getSpecies()[2], "B");
  EXPECT_EQ(reaction.getSpecies()[3], "D");
  EXPECT_EQ(reaction.getReactantSpecies()[0], "A");
  EXPECT_EQ(reaction.getReactantSpecies()[1], "C");
  EXPECT_EQ(reaction.getProductSpecies()[0], "B");
  EXPECT_EQ(reaction.getProductSpecies()[1], "D");
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[0], -10);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[1], 3.4);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[2], -2.1111111);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[3], 24);
  EXPECT_EQ(reaction.hasMetaData("this"), false);
  EXPECT_EQ(reaction.hasMetaData<Real>("k"), true);
  EXPECT_EQ(reaction.hasMetaData("k"), true);
  EXPECT_EQ(reaction.getMetaData("k"), "2.1");
  EXPECT_EQ(reaction.hasMetaData<Real>("Q"), true);
  EXPECT_EQ(reaction.hasMetaData("Q"), true);
  EXPECT_EQ(reaction.getMetaData("Q"), "-2");

  // One string metadata
  reaction = ReactionNetworkUtils::parseReactionNetwork(
      "-10A + 3.4C -> -2.1111111B + 24D [k=2.1, Q=-2, P=there]", false)[0];
  EXPECT_EQ(reaction.getSpecies()[0], "A");
  EXPECT_EQ(reaction.getSpecies()[1], "C");
  EXPECT_EQ(reaction.getSpecies()[2], "B");
  EXPECT_EQ(reaction.getSpecies()[3], "D");
  EXPECT_EQ(reaction.getReactantSpecies()[0], "A");
  EXPECT_EQ(reaction.getReactantSpecies()[1], "C");
  EXPECT_EQ(reaction.getProductSpecies()[0], "B");
  EXPECT_EQ(reaction.getProductSpecies()[1], "D");
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[0], -10);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[1], 3.4);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[2], -2.1111111);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[3], 24);
  EXPECT_EQ(reaction.hasMetaData("this"), false);
  EXPECT_EQ(reaction.hasMetaData<Real>("k"), true);
  EXPECT_EQ(reaction.hasMetaData("k"), true);
  EXPECT_EQ(reaction.getMetaData("k"), "2.1");
  EXPECT_EQ(reaction.hasMetaData<Real>("Q"), true);
  EXPECT_EQ(reaction.hasMetaData("Q"), true);
  EXPECT_EQ(reaction.getMetaData("Q"), "-2");
  EXPECT_EQ(reaction.hasMetaData<Real>("P"), false);
  EXPECT_EQ(reaction.hasMetaData("P"), true);
  EXPECT_EQ(reaction.getMetaData("P"), "there");

  // Two reactions
  auto reactions = ReactionNetworkUtils::parseReactionNetwork(
      "-A + C -> -B [K=23,Q=2]\nD(aq) + E+ -> F - 1.1G[S12=meta,log34=three]", false);
  reaction = reactions[0];
  EXPECT_EQ(reaction.getSpecies()[0], "A");
  EXPECT_EQ(reaction.getSpecies()[1], "C");
  EXPECT_EQ(reaction.getSpecies()[2], "B");
  EXPECT_EQ(reaction.getReactantSpecies()[0], "A");
  EXPECT_EQ(reaction.getReactantSpecies()[1], "C");
  EXPECT_EQ(reaction.getProductSpecies()[0], "B");
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[0], -1);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[1], 1);
  EXPECT_EQ(reaction.getStoichiometricCoefficients()[2], -1);
  EXPECT_EQ(reaction.hasMetaData("this"), false);
  auto reaction2 = reactions[1];
  EXPECT_EQ(reaction2.getSpecies()[0], "D(aq)");
  EXPECT_EQ(reaction2.getSpecies()[1], "E+");
  EXPECT_EQ(reaction2.getSpecies()[2], "F");
  EXPECT_EQ(reaction2.getSpecies()[3], "G");
  EXPECT_EQ(reaction2.getReactantSpecies()[0], "D(aq)");
  EXPECT_EQ(reaction2.getReactantSpecies()[1], "E+");
  EXPECT_EQ(reaction2.getProductSpecies()[0], "F");
  EXPECT_EQ(reaction2.getProductSpecies()[1], "G");
  EXPECT_EQ(reaction2.getStoichiometricCoefficients()[0], 1);
  EXPECT_EQ(reaction2.getStoichiometricCoefficients()[1], 1);
  EXPECT_EQ(reaction2.getStoichiometricCoefficients()[2], 1);
  EXPECT_EQ(reaction2.getStoichiometricCoefficients()[3], -1.1);
  EXPECT_EQ(reaction2.hasMetaData("this"), false);
}
