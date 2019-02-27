#ifndef NUMERICSTEST_H
#define NUMERICSTEST_H

#include "MooseObjectUnitTest.h"

/**
 * Tests numerics utilities
 */
class NumericsTest : public MooseObjectUnitTest
{
public:
  NumericsTest() : MooseObjectUnitTest("THMTestApp") {}
};

#endif /* NUMERICSTEST_H */
