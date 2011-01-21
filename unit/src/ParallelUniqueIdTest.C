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

// #include "ParallelUniqueIdTest.h"

// //Moose includes
// #include "ParallelUniqueId.h"

// CPPUNIT_TEST_SUITE_REGISTRATION( ParallelUniqueIdTest );


// void
// ParallelUniqueIdTest::constructor()
// {
//   ParallelUniqueId::initialize();
//   ParallelUniqueId puid;

//   //first id should be 0 regardless of whether LIBMESH_HAVE_TBB_API is defined
//   CPPUNIT_ASSERT( puid.id == 0 );
// }

// void
// ParallelUniqueIdTest::testId()
// {
//   ParallelUniqueId::initialize();
//   ParallelUniqueId puid1;
//   ParallelUniqueId puid2;

//   #ifdef LIBMESH_HAVE_TBB_API
//     CPPUNIT_ASSERT( puid1.id == 0 );
//     CPPUNIT_ASSERT( puid2.id == 1 );
//   #else
//     CPPUNIT_ASSERT( puid1.id == 0 );
//     CPPUNIT_ASSERT( puid2.id == 0 );
//   #endif
// }

