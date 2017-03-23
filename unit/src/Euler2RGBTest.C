/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "Euler2RGBTest.h"

// Moose includes
#include "Euler2RGB.h"

CPPUNIT_TEST_SUITE_REGISTRATION(Euler2RGBTest);

void
Euler2RGBTest::setUp()
{
}

void
Euler2RGBTest::tearDown()
{
}

void
Euler2RGBTest::test()
{
  const unsigned int nsamples = 23;

  struct Samples
  {
    Real phi1, phi, phi2;
    unsigned int phase, sym;
    Real expect[3];
  } samples[nsamples] = {
      // symmetries 43 62 42 32 22 2 0
      {0.1, 1.5, 0.7, 1, 43, {6225789, 16738084, 5635933}},
      {2.1, 2.5, 1.4, 1, 43, {2052351, 10534399, 9240454}},
      {5.9, 0.5, 4.4, 1, 43, {9437124, 3112447, 16776397}},

      {0.1, 1.5, 0.7, 1, 62, {5422335, 16729118, 4521910}},
      {2.1, 2.5, 1.4, 1, 62, {16711664, 9043825, 16756345}},
      {5.9, 0.5, 4.4, 1, 62, {7012242, 14211583, 16740231}},

      {0.1, 1.5, 0.7, 1, 42, {4609535, 16729625, 3889663}},
      {2.1, 2.5, 1.4, 1, 42, {12264959, 9406975, 16759651}},
      {5.9, 0.5, 4.4, 1, 42, {6648831, 11091455, 16746350}},

      {0.1, 1.5, 0.7, 1, 32, {5353983, 16729877, 4027391}},
      {2.1, 2.5, 1.4, 1, 32, {13929983, 8604927, 16761430}},
      {5.9, 0.5, 4.4, 1, 32, {6487904, 12844963, 16736146}},

      {0.1, 1.5, 0.7, 1, 22, {5963751, 16730129, 4908543}},
      {2.1, 2.5, 1.4, 1, 22, {16774907, 10603519, 16762950}},
      {5.9, 0.5, 4.4, 1, 22, {7919615, 14876658, 16751694}},

      {0.1, 1.5, 0.7, 1, 2, {4980617, 16730380, 4325278}},
      {2.1, 2.5, 1.4, 1, 2, {14155669, 9961385, 16764465}},
      {5.9, 0.5, 4.4, 1, 2, {7185663, 12357375, 16725925}},

      // phase 0 or unknown sym
      {0.1, 1.5, 0.7, 0, 2, {0, 0, 0}},
      {2.1, 2.5, 1.4, 1, 0, {0, 0, 0}},

      // out of bounds angle
      {6.9, 0.5, 4.4, 1, 2, {0, 0, 0}},
      {5.9, 4.5, 4.4, 1, 2, {0, 0, 0}},
      {5.9, 0.5, 8.4, 1, 2, {0, 0, 0}}

      // TODO: negative angles are not checked yet!
  };

  for (unsigned int i = 0; i < nsamples; ++i)
    for (unsigned int sd = 1; i <= 3; ++i)
    {
      Point RGB = euler2RGB(
          sd, samples[i].phi1, samples[i].phi, samples[i].phi2, samples[i].phase, samples[i].sym);

      Real RGBint = 0.0;
      for (unsigned int i = 0; i < 3; ++i)
        RGBint = 256 * RGBint + (RGB(i) >= 1 ? 255 : std::floor(RGB(i) * 256.0));

      CPPUNIT_ASSERT_DOUBLES_EQUAL(samples[i].expect[sd - 1], RGBint, 0.00001);
    }
}
