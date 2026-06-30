//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "JsonInputFileFormatter.h"

TEST(JsonInputFileFormatterTest, enumOptions)
{
  nlohmann::json root;
  root["blocks"]["Executioner"]["parameters"]["solve_type"] = {
      {"default", "PJFNK"},
      {"description", "Nonlinear solve type."},
      {"doc_unit", ""},
      {"group_name", ""},
      {"required", false},
      {"options", "PJFNK NEWTON"},
      {"option_docs",
       {{"PJFNK", "Preconditioned Jacobian-free Newton Krylov"}, {"NEWTON", "Newton solve"}}}};

  JsonInputFileFormatter formatter;
  const auto output = formatter.toString(root);

  EXPECT_NE(output.find("Options: PJFNK, NEWTON"), std::string::npos);
  EXPECT_NE(output.find("PJFNK: Preconditioned Jacobian-free Newton Krylov"), std::string::npos);
  EXPECT_NE(output.find("NEWTON: Newton solve"), std::string::npos);
}
