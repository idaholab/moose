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

#include "MemDataTest.h"

// Moose includes
#include "Moose.h" // Real
#include "MemData.h"

// This macro declares a static variable whose construction
// causes a test suite factory to be inserted in a global registry
// of such factories.
CPPUNIT_TEST_SUITE_REGISTRATION( MemDataTest );


void MemDataTest::test()
{
  MemData mem_data;

  // Begin logging
  mem_data.start();

  // Allocate memory
  unsigned array_size = 1024*1024; // 4 bytes/int * 1M = 4M
  int* dummy_memory = new int[array_size];

  // Just calling "new" alone does not register any change in the
  // operating system...you actually have to touch all the memory
  // you asked for.
  for (unsigned i=0; i<array_size; ++i)
    dummy_memory[i] = i;

  // Stop logging
  mem_data.stop();

  // Compute result
  long mem_used = mem_data.delta();
  //std::cout << "mem_used=" << mem_used << std::endl;
  Real megs_used = static_cast<Real>(mem_used)/static_cast<Real>(1024);

  // Assert that the memory counter counted *at least* 4 Megabytes
  CPPUNIT_ASSERT(megs_used >= 4.);

  Real rel_diff = std::abs(megs_used-4.)/4.;
  //std::cout << "rel_diff=" << rel_diff << std::endl;

  // Assert that the memory counter counted *close* to 4 Megabytes
  CPPUNIT_ASSERT( rel_diff < 1.e-2 );

  // Clean up the dummy array we created
  delete dummy_memory;
}
