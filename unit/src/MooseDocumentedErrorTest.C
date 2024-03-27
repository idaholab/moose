//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseDocumentedErrorTest.h"

TEST_F(MooseDocumentedErrorTest, mooseObjectError)
{
  EXPECT_THROW(
      {
        try
        {
          _fe_problem->mooseDocumentedError("moose", 1234, "foo");
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "The following error occurred in the Problem 'problem' of type "
                    "FEProblem.\n\nfoo\n\nThis error is documented at "
                    "github.com/idaholab/moose/issues/1234.");
          throw;
        }
      },
      std::exception);
}

TEST_F(MooseDocumentedErrorTest, staticError)
{
  EXPECT_THROW(
      {
        try
        {
          ::mooseDocumentedError("moose", 5678, "bar");
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "bar\n\nThis error is documented at github.com/idaholab/moose/issues/5678.");
          throw;
        }
      },
      std::exception);
}
