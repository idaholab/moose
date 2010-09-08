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

#ifndef MOOSEARRAYTEST_H
#define MOOSEARRAYTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

class MooseArrayTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE( MooseArrayTest );

  CPPUNIT_TEST( defaultConstructor );
  CPPUNIT_TEST( sizeConstructor );
  CPPUNIT_TEST( valueConstructor );
  CPPUNIT_TEST( setAllValues );
  CPPUNIT_TEST( release );
  CPPUNIT_TEST( resize );
  CPPUNIT_TEST( resizeDefault );
  CPPUNIT_TEST( size );
  CPPUNIT_TEST( access );
  CPPUNIT_TEST( shallowCopy );

  CPPUNIT_TEST_SUITE_END();
  
public:
  void setUp();
  void tearDown();

  void defaultConstructor();
  void sizeConstructor();
  void valueConstructor();
  void setAllValues();
  void release();
  void resize();
  void resizeDefault();
  void size();
  void access();
  void shallowCopy();

private:
};

#endif  // MOOSEARRAYTEST_H
