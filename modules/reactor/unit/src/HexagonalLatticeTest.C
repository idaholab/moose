//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HexagonalLatticeTest.h"

TEST_F(HexagonalLatticeTest, rings_and_pins)
{
  Real bp = 10.0, pp = 0.1, pd = 0.04, wd = 0.01, wp = 50.0;
  unsigned int nr = 1, a = 2;
  HexagonalLatticeUtils hl1(bp, pp, pd, wd, wp, nr, a);

  // Test number of rings given number of pins
  EXPECT_EQ(hl1.rings(1), (unsigned int)1);
  EXPECT_EQ(hl1.rings(7), (unsigned int)2);
  EXPECT_EQ(hl1.rings(19), (unsigned int)3);
  EXPECT_EQ(hl1.rings(37), (unsigned int)4);

  try
  {
    hl1.rings(100);
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("Number of pins 100 not evenly divisible in a hexagonal lattice!"),
              std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  // Test number of pins in given ring
  EXPECT_EQ(hl1.pins(1), (unsigned int)1);
  EXPECT_EQ(hl1.pins(2), (unsigned int)6);
  EXPECT_EQ(hl1.pins(3), (unsigned int)12);

  // Test first and last pins in given ring
  EXPECT_EQ(hl1.firstPinInRing(1), (unsigned int)0);
  EXPECT_EQ(hl1.lastPinInRing(1), (unsigned int)0);

  EXPECT_EQ(hl1.firstPinInRing(2), (unsigned int)1);
  EXPECT_EQ(hl1.lastPinInRing(2), (unsigned int)6);

  EXPECT_EQ(hl1.firstPinInRing(3), (unsigned int)7);
  EXPECT_EQ(hl1.lastPinInRing(3), (unsigned int)18);
}

TEST_F(HexagonalLatticeTest, variation_with_rings)
{
  Real bp = 10.0, pp = 0.1, pd = 0.04, wd = 0.01, wp = 50.0;
  unsigned int nr = 1, a = 2;
  HexagonalLatticeUtils hl1(bp, pp, pd, wd, wp, nr, a);

  nr = 2;
  HexagonalLatticeUtils hl2(bp, pp, pd, wd, wp, nr, a);

  nr = 3;
  HexagonalLatticeUtils hl3(bp, pp, pd, wd, wp, nr, a);

  nr = 4;
  HexagonalLatticeUtils hl4(bp, pp, pd, wd, wp, nr, a);

  EXPECT_EQ(hl1.nPins(), (unsigned int)1);
  EXPECT_EQ(hl1.nInteriorPins(), (unsigned int)1);
  EXPECT_EQ(hl1.nEdgePins(), (unsigned int)0);
  EXPECT_EQ(hl1.nCornerPins(), (unsigned int)0);
  EXPECT_EQ(hl1.nChannels(), (unsigned int)6);
  EXPECT_EQ(hl1.nInteriorChannels(), (unsigned int)0);
  EXPECT_EQ(hl1.nEdgeChannels(), (unsigned int)0);
  EXPECT_EQ(hl1.nCornerChannels(), (unsigned int)6);

  EXPECT_EQ(hl2.nPins(), (unsigned int)7);
  EXPECT_EQ(hl2.nInteriorPins(), (unsigned int)1);
  EXPECT_EQ(hl2.nEdgePins(), (unsigned int)0);
  EXPECT_EQ(hl2.nCornerPins(), (unsigned int)6);
  EXPECT_EQ(hl2.nChannels(), (unsigned int)18);
  EXPECT_EQ(hl2.nInteriorChannels(), (unsigned int)6);
  EXPECT_EQ(hl2.nEdgeChannels(), (unsigned int)6);
  EXPECT_EQ(hl2.nCornerChannels(), (unsigned int)6);

  EXPECT_EQ(hl3.nPins(), (unsigned int)19);
  EXPECT_EQ(hl3.nInteriorPins(), (unsigned int)7);
  EXPECT_EQ(hl3.nEdgePins(), (unsigned int)6);
  EXPECT_EQ(hl3.nCornerPins(), (unsigned int)6);
  EXPECT_EQ(hl3.nChannels(), (unsigned int)42);
  EXPECT_EQ(hl3.nInteriorChannels(), (unsigned int)24);
  EXPECT_EQ(hl3.nEdgeChannels(), (unsigned int)12);
  EXPECT_EQ(hl3.nCornerChannels(), (unsigned int)6);

  EXPECT_EQ(hl4.nPins(), (unsigned int)37);
  EXPECT_EQ(hl4.nInteriorPins(), (unsigned int)19);
  EXPECT_EQ(hl4.nEdgePins(), (unsigned int)12);
  EXPECT_EQ(hl4.nCornerPins(), (unsigned int)6);
  EXPECT_EQ(hl4.nChannels(), (unsigned int)78);
  EXPECT_EQ(hl4.nInteriorChannels(), (unsigned int)54);
  EXPECT_EQ(hl4.nEdgeChannels(), (unsigned int)18);
  EXPECT_EQ(hl4.nCornerChannels(), (unsigned int)6);

  auto interior_pins2 = hl2.interiorChannelPinIndices();
  EXPECT_EQ(interior_pins2[0][0], (unsigned int)0);
  EXPECT_EQ(interior_pins2[0][1], (unsigned int)1);
  EXPECT_EQ(interior_pins2[0][2], (unsigned int)2);
  EXPECT_EQ(interior_pins2[1][0], (unsigned int)0);
  EXPECT_EQ(interior_pins2[1][1], (unsigned int)2);
  EXPECT_EQ(interior_pins2[1][2], (unsigned int)3);
  EXPECT_EQ(interior_pins2[2][0], (unsigned int)0);
  EXPECT_EQ(interior_pins2[2][1], (unsigned int)3);
  EXPECT_EQ(interior_pins2[2][2], (unsigned int)4);
  EXPECT_EQ(interior_pins2[3][0], (unsigned int)0);
  EXPECT_EQ(interior_pins2[3][1], (unsigned int)4);
  EXPECT_EQ(interior_pins2[3][2], (unsigned int)5);
  EXPECT_EQ(interior_pins2[4][0], (unsigned int)0);
  EXPECT_EQ(interior_pins2[4][1], (unsigned int)5);
  EXPECT_EQ(interior_pins2[4][2], (unsigned int)6);
  EXPECT_EQ(interior_pins2[5][0], (unsigned int)0);
  EXPECT_EQ(interior_pins2[5][1], (unsigned int)6);
  EXPECT_EQ(interior_pins2[5][2], (unsigned int)1);

  auto edge_pins2 = hl2.edgeChannelPinIndices();
  EXPECT_EQ(edge_pins2[0][0], (unsigned int)1);
  EXPECT_EQ(edge_pins2[0][1], (unsigned int)2);
  EXPECT_EQ(edge_pins2[1][0], (unsigned int)2);
  EXPECT_EQ(edge_pins2[1][1], (unsigned int)3);
  EXPECT_EQ(edge_pins2[2][0], (unsigned int)3);
  EXPECT_EQ(edge_pins2[2][1], (unsigned int)4);
  EXPECT_EQ(edge_pins2[3][0], (unsigned int)4);
  EXPECT_EQ(edge_pins2[3][1], (unsigned int)5);
  EXPECT_EQ(edge_pins2[4][0], (unsigned int)5);
  EXPECT_EQ(edge_pins2[4][1], (unsigned int)6);
  EXPECT_EQ(edge_pins2[5][0], (unsigned int)6);
  EXPECT_EQ(edge_pins2[5][1], (unsigned int)1);

  auto corner_pins2 = hl2.cornerChannelPinIndices();
  EXPECT_EQ(corner_pins2[0][0], (unsigned int)1);
  EXPECT_EQ(corner_pins2[1][0], (unsigned int)2);
  EXPECT_EQ(corner_pins2[2][0], (unsigned int)3);
  EXPECT_EQ(corner_pins2[3][0], (unsigned int)4);
  EXPECT_EQ(corner_pins2[4][0], (unsigned int)5);
  EXPECT_EQ(corner_pins2[5][0], (unsigned int)6);

  auto interior_pins3 = hl3.interiorChannelPinIndices();
  EXPECT_EQ(interior_pins3[0][0], (unsigned int)0);
  EXPECT_EQ(interior_pins3[0][1], (unsigned int)1);
  EXPECT_EQ(interior_pins3[0][2], (unsigned int)2);
  EXPECT_EQ(interior_pins3[1][0], (unsigned int)0);
  EXPECT_EQ(interior_pins3[1][1], (unsigned int)2);
  EXPECT_EQ(interior_pins3[1][2], (unsigned int)3);
  EXPECT_EQ(interior_pins3[2][0], (unsigned int)0);
  EXPECT_EQ(interior_pins3[2][1], (unsigned int)3);
  EXPECT_EQ(interior_pins3[2][2], (unsigned int)4);
  EXPECT_EQ(interior_pins3[3][0], (unsigned int)0);
  EXPECT_EQ(interior_pins3[3][1], (unsigned int)4);
  EXPECT_EQ(interior_pins3[3][2], (unsigned int)5);
  EXPECT_EQ(interior_pins3[4][0], (unsigned int)0);
  EXPECT_EQ(interior_pins3[4][1], (unsigned int)5);
  EXPECT_EQ(interior_pins3[4][2], (unsigned int)6);
  EXPECT_EQ(interior_pins3[5][0], (unsigned int)0);
  EXPECT_EQ(interior_pins3[5][1], (unsigned int)6);
  EXPECT_EQ(interior_pins3[5][2], (unsigned int)1);
  EXPECT_EQ(interior_pins3[6][0], (unsigned int)1);
  EXPECT_EQ(interior_pins3[6][1], (unsigned int)7);
  EXPECT_EQ(interior_pins3[6][2], (unsigned int)8);
  EXPECT_EQ(interior_pins3[7][0], (unsigned int)8);
  EXPECT_EQ(interior_pins3[7][1], (unsigned int)2);
  EXPECT_EQ(interior_pins3[7][2], (unsigned int)1);
  EXPECT_EQ(interior_pins3[8][0], (unsigned int)2);
  EXPECT_EQ(interior_pins3[8][1], (unsigned int)8);
  EXPECT_EQ(interior_pins3[8][2], (unsigned int)9);
  EXPECT_EQ(interior_pins3[9][0], (unsigned int)2);
  EXPECT_EQ(interior_pins3[9][1], (unsigned int)9);
  EXPECT_EQ(interior_pins3[9][2], (unsigned int)10);
  EXPECT_EQ(interior_pins3[10][0], (unsigned int)10);
  EXPECT_EQ(interior_pins3[10][1], (unsigned int)3);
  EXPECT_EQ(interior_pins3[10][2], (unsigned int)2);
  EXPECT_EQ(interior_pins3[11][0], (unsigned int)3);
  EXPECT_EQ(interior_pins3[11][1], (unsigned int)10);
  EXPECT_EQ(interior_pins3[11][2], (unsigned int)11);
  EXPECT_EQ(interior_pins3[12][0], (unsigned int)3);
  EXPECT_EQ(interior_pins3[12][1], (unsigned int)11);
  EXPECT_EQ(interior_pins3[12][2], (unsigned int)12);
  EXPECT_EQ(interior_pins3[13][0], (unsigned int)12);
  EXPECT_EQ(interior_pins3[13][1], (unsigned int)4);
  EXPECT_EQ(interior_pins3[13][2], (unsigned int)3);
  EXPECT_EQ(interior_pins3[14][0], (unsigned int)4);
  EXPECT_EQ(interior_pins3[14][1], (unsigned int)12);
  EXPECT_EQ(interior_pins3[14][2], (unsigned int)13);
  EXPECT_EQ(interior_pins3[15][0], (unsigned int)4);
  EXPECT_EQ(interior_pins3[15][1], (unsigned int)13);
  EXPECT_EQ(interior_pins3[15][2], (unsigned int)14);
  EXPECT_EQ(interior_pins3[16][0], (unsigned int)14);
  EXPECT_EQ(interior_pins3[16][1], (unsigned int)5);
  EXPECT_EQ(interior_pins3[16][2], (unsigned int)4);
  EXPECT_EQ(interior_pins3[17][0], (unsigned int)5);
  EXPECT_EQ(interior_pins3[17][1], (unsigned int)14);
  EXPECT_EQ(interior_pins3[17][2], (unsigned int)15);
  EXPECT_EQ(interior_pins3[18][0], (unsigned int)5);
  EXPECT_EQ(interior_pins3[18][1], (unsigned int)15);
  EXPECT_EQ(interior_pins3[18][2], (unsigned int)16);
  EXPECT_EQ(interior_pins3[19][0], (unsigned int)16);
  EXPECT_EQ(interior_pins3[19][1], (unsigned int)6);
  EXPECT_EQ(interior_pins3[19][2], (unsigned int)5);
  EXPECT_EQ(interior_pins3[20][0], (unsigned int)6);
  EXPECT_EQ(interior_pins3[20][1], (unsigned int)16);
  EXPECT_EQ(interior_pins3[20][2], (unsigned int)17);
  EXPECT_EQ(interior_pins3[21][0], (unsigned int)6);
  EXPECT_EQ(interior_pins3[21][1], (unsigned int)17);
  EXPECT_EQ(interior_pins3[21][2], (unsigned int)18);
  EXPECT_EQ(interior_pins3[22][0], (unsigned int)18);
  EXPECT_EQ(interior_pins3[22][1], (unsigned int)1);
  EXPECT_EQ(interior_pins3[22][2], (unsigned int)6);
  EXPECT_EQ(interior_pins3[23][0], (unsigned int)1);
  EXPECT_EQ(interior_pins3[23][1], (unsigned int)18);
  EXPECT_EQ(interior_pins3[23][2], (unsigned int)7);

  auto edge_pins3 = hl3.edgeChannelPinIndices();
  EXPECT_EQ(edge_pins3[0][0], (unsigned int)7);
  EXPECT_EQ(edge_pins3[0][1], (unsigned int)8);
  EXPECT_EQ(edge_pins3[1][0], (unsigned int)8);
  EXPECT_EQ(edge_pins3[1][1], (unsigned int)9);
  EXPECT_EQ(edge_pins3[2][0], (unsigned int)9);
  EXPECT_EQ(edge_pins3[2][1], (unsigned int)10);
  EXPECT_EQ(edge_pins3[3][0], (unsigned int)10);
  EXPECT_EQ(edge_pins3[3][1], (unsigned int)11);
  EXPECT_EQ(edge_pins3[4][0], (unsigned int)11);
  EXPECT_EQ(edge_pins3[4][1], (unsigned int)12);
  EXPECT_EQ(edge_pins3[5][0], (unsigned int)12);
  EXPECT_EQ(edge_pins3[5][1], (unsigned int)13);
  EXPECT_EQ(edge_pins3[6][0], (unsigned int)13);
  EXPECT_EQ(edge_pins3[6][1], (unsigned int)14);
  EXPECT_EQ(edge_pins3[7][0], (unsigned int)14);
  EXPECT_EQ(edge_pins3[7][1], (unsigned int)15);
  EXPECT_EQ(edge_pins3[8][0], (unsigned int)15);
  EXPECT_EQ(edge_pins3[8][1], (unsigned int)16);
  EXPECT_EQ(edge_pins3[9][0], (unsigned int)16);
  EXPECT_EQ(edge_pins3[9][1], (unsigned int)17);
  EXPECT_EQ(edge_pins3[10][0], (unsigned int)17);
  EXPECT_EQ(edge_pins3[10][1], (unsigned int)18);
  EXPECT_EQ(edge_pins3[11][0], (unsigned int)18);
  EXPECT_EQ(edge_pins3[11][1], (unsigned int)7);

  auto corner_pins3 = hl3.cornerChannelPinIndices();
  EXPECT_EQ(corner_pins3[0][0], (unsigned int)7);
  EXPECT_EQ(corner_pins3[1][0], (unsigned int)9);
  EXPECT_EQ(corner_pins3[2][0], (unsigned int)11);
  EXPECT_EQ(corner_pins3[3][0], (unsigned int)13);
  EXPECT_EQ(corner_pins3[4][0], (unsigned int)15);
  EXPECT_EQ(corner_pins3[5][0], (unsigned int)17);
}

TEST_F(HexagonalLatticeTest, pin_bundle_spacing)
{
  Real bp = 5.0, pp = 1.0, pd = 0.8, wd = 0.05, wp = 50.0;
  unsigned int nr = 1, a = 2;
  HexagonalLatticeUtils hl5(bp, pp, pd, wd, wp, nr, a);

  nr = 2;
  HexagonalLatticeUtils hl6(bp, pp, pd, wd, wp, nr, a);

  wd = 0.0;
  nr = 1;
  HexagonalLatticeUtils hl7(bp, pp, pd, wd, wp, nr, a);

  nr = 2;
  HexagonalLatticeUtils hl8(bp, pp, pd, wd, wp, nr, a);

  // wire-wrapped bundles
  EXPECT_DOUBLE_EQ(hl5.pinBundleSpacing(), 2.1);
  EXPECT_DOUBLE_EQ(hl6.pinBundleSpacing(), (5.0 - (std::sqrt(3.0) + 0.8)) / 2.);

  // bare bundles
  EXPECT_DOUBLE_EQ(hl7.pinBundleSpacing(), 2.1);
  EXPECT_DOUBLE_EQ(hl8.pinBundleSpacing(), (5.0 - (std::sqrt(3.0) + 0.8)) / 2.);
}

TEST_F(HexagonalLatticeTest, hydraulic_diameters)
{
  int n_interior, n_edge, n_corner;
  Real flow_interior, flow_edge, flow_corner;
  Real area_interior, area_edge, area_corner;

  Real bp = 5.0, pp = 1.0, pd = 0.8, wd = 0.1, wp = 50.0;
  unsigned int nr = 1, a = 2;
  HexagonalLatticeUtils hl9(bp, pp, pd, wd, wp, nr, a);

  nr = 2;
  HexagonalLatticeUtils hl10(bp, pp, pd, wd, wp, nr, a);

  pp = 0.99;
  nr = 3;
  HexagonalLatticeUtils hl11(bp, pp, pd, wd, wp, nr, a);

  pp = 1.0;
  wd = 0.0;
  nr = 1;
  HexagonalLatticeUtils hl12(bp, pp, pd, wd, wp, nr, a);

  nr = 2;
  HexagonalLatticeUtils hl13(bp, pp, pd, wd, wp, nr, a);

  pp = 0.99;
  nr = 3;
  HexagonalLatticeUtils hl14(bp, pp, pd, wd, wp, nr, a);

  // values are obtained from a completely separately-developed Python script for
  // verifying the bundle-wide hydraulic diameter. We verify the diameters for each
  // individual channel by requiring that the n-channel-weighted sum equals the
  // bundle-wide values for volume and areas, since we can verify that easily.
  EXPECT_DOUBLE_EQ(hl9.hydraulicDiameter(), 4.196872851813099);
  n_interior = hl9.nInteriorChannels();
  n_edge = hl9.nEdgeChannels();
  n_corner = hl9.nCornerChannels();
  flow_interior = hl9.interiorFlowVolume();
  flow_edge = hl9.edgeFlowVolume();
  flow_corner = hl9.cornerFlowVolume();
  area_interior = hl9.interiorWettedArea();
  area_edge = hl9.edgeWettedArea();
  area_corner = hl9.cornerWettedArea();
  EXPECT_DOUBLE_EQ(hl9.flowVolume(),
                   n_interior * flow_interior + n_edge * flow_edge + n_corner * flow_corner);
  EXPECT_DOUBLE_EQ(hl9.wettedArea(),
                   n_interior * area_interior + n_edge * area_edge + n_corner * area_corner);

  EXPECT_DOUBLE_EQ(hl10.hydraulicDiameter(), 1.948158075579034);
  n_interior = hl10.nInteriorChannels();
  n_edge = hl10.nEdgeChannels();
  n_corner = hl10.nCornerChannels();
  flow_interior = hl10.interiorFlowVolume();
  flow_edge = hl10.edgeFlowVolume();
  flow_corner = hl10.cornerFlowVolume();
  area_interior = hl10.interiorWettedArea();
  area_edge = hl10.edgeWettedArea();
  area_corner = hl10.cornerWettedArea();
  EXPECT_DOUBLE_EQ(hl10.flowVolume(),
                   n_interior * flow_interior + n_edge * flow_edge + n_corner * flow_corner);
  EXPECT_DOUBLE_EQ(hl10.wettedArea(),
                   n_interior * area_interior + n_edge * area_edge + n_corner * area_corner);

  EXPECT_DOUBLE_EQ(hl11.hydraulicDiameter(), 0.6727946134052672);
  n_interior = hl11.nInteriorChannels();
  n_edge = hl11.nEdgeChannels();
  n_corner = hl11.nCornerChannels();
  flow_interior = hl11.interiorFlowVolume();
  flow_edge = hl11.edgeFlowVolume();
  flow_corner = hl11.cornerFlowVolume();
  area_interior = hl11.interiorWettedArea();
  area_edge = hl11.edgeWettedArea();
  area_corner = hl11.cornerWettedArea();
  EXPECT_DOUBLE_EQ(hl11.flowVolume(),
                   n_interior * flow_interior + n_edge * flow_edge + n_corner * flow_corner);
  EXPECT_DOUBLE_EQ(hl11.wettedArea(),
                   n_interior * area_interior + n_edge * area_edge + n_corner * area_corner);

  // bare bundles
  EXPECT_DOUBLE_EQ(hl12.hydraulicDiameter(), 4.2650423521483205);
  n_interior = hl12.nInteriorChannels();
  n_edge = hl12.nEdgeChannels();
  n_corner = hl12.nCornerChannels();
  flow_interior = hl12.interiorFlowVolume();
  flow_edge = hl12.edgeFlowVolume();
  flow_corner = hl12.cornerFlowVolume();
  area_interior = hl12.interiorWettedArea();
  area_edge = hl12.edgeWettedArea();
  area_corner = hl12.cornerWettedArea();
  EXPECT_DOUBLE_EQ(hl12.flowVolume(),
                   n_interior * flow_interior + n_edge * flow_edge + n_corner * flow_corner);
  EXPECT_DOUBLE_EQ(hl12.wettedArea(),
                   n_interior * area_interior + n_edge * area_edge + n_corner * area_corner);

  EXPECT_DOUBLE_EQ(hl13.hydraulicDiameter(), 2.077372852104904);
  n_interior = hl13.nInteriorChannels();
  n_edge = hl13.nEdgeChannels();
  n_corner = hl13.nCornerChannels();
  flow_interior = hl13.interiorFlowVolume();
  flow_edge = hl13.edgeFlowVolume();
  flow_corner = hl13.cornerFlowVolume();
  area_interior = hl13.interiorWettedArea();
  area_edge = hl13.edgeWettedArea();
  area_corner = hl13.cornerWettedArea();
  EXPECT_DOUBLE_EQ(hl13.flowVolume(),
                   n_interior * flow_interior + n_edge * flow_edge + n_corner * flow_corner);
  EXPECT_DOUBLE_EQ(hl13.wettedArea(),
                   n_interior * area_interior + n_edge * area_edge + n_corner * area_corner);

  EXPECT_DOUBLE_EQ(hl14.hydraulicDiameter(), 0.7437951937590452);
  n_interior = hl14.nInteriorChannels();
  n_edge = hl14.nEdgeChannels();
  n_corner = hl14.nCornerChannels();
  flow_interior = hl14.interiorFlowVolume();
  flow_edge = hl14.edgeFlowVolume();
  flow_corner = hl14.cornerFlowVolume();
  area_interior = hl14.interiorWettedArea();
  area_edge = hl14.edgeWettedArea();
  area_corner = hl14.cornerWettedArea();
  EXPECT_DOUBLE_EQ(hl14.flowVolume(),
                   n_interior * flow_interior + n_edge * flow_edge + n_corner * flow_corner);
  EXPECT_DOUBLE_EQ(hl14.wettedArea(),
                   n_interior * area_interior + n_edge * area_edge + n_corner * area_corner);
}

TEST_F(HexagonalLatticeTest, pin_centers)
{
  Real bp = 5.0, pp = 0.99, pd = 0.8, wd = 0.0, wp = 50.0;
  unsigned int nr = 3, a = 2;
  HexagonalLatticeUtils hl14(bp, pp, pd, wd, wp, nr, a);

  nr = 1;
  HexagonalLatticeUtils hl15(bp, pp, pd, wd, wp, nr, a);

  nr = 2;
  HexagonalLatticeUtils hl16(bp, pp, pd, wd, wp, nr, a);

  bp = 10.0;
  nr = 4;
  HexagonalLatticeUtils hl17(bp, pp, pd, wd, wp, nr, a);

  Real cos60 = 0.5;
  Real sin60 = std::sqrt(3.0) / 2.0;

  auto & centers = hl15.pinCenters();
  EXPECT_EQ(centers.size(), hl15.nPins());
  EXPECT_DOUBLE_EQ(centers[0](0), 0.0);
  EXPECT_DOUBLE_EQ(centers[0](1), 0.0);
  EXPECT_DOUBLE_EQ(centers[0](2), 0.0);

  Real p = 0.99;
  auto & centers2 = hl16.pinCenters();
  EXPECT_EQ(centers2.size(), hl16.nPins());
  EXPECT_DOUBLE_EQ(centers2[0](0), 0.0);
  EXPECT_DOUBLE_EQ(centers2[0](1), 0.0);
  EXPECT_DOUBLE_EQ(centers2[1](0), cos60 * p);
  EXPECT_DOUBLE_EQ(centers2[1](1), sin60 * p);
  EXPECT_DOUBLE_EQ(centers2[2](0), -cos60 * p);
  EXPECT_DOUBLE_EQ(centers2[2](1), sin60 * p);
  EXPECT_DOUBLE_EQ(centers2[3](0), -p);
  EXPECT_DOUBLE_EQ(centers2[3](1), 0.0);
  EXPECT_DOUBLE_EQ(centers2[4](0), -cos60 * p);
  EXPECT_DOUBLE_EQ(centers2[4](1), -sin60 * p);
  EXPECT_DOUBLE_EQ(centers2[5](0), cos60 * p);
  EXPECT_DOUBLE_EQ(centers2[5](1), -sin60 * p);
  EXPECT_DOUBLE_EQ(centers2[6](0), p);
  EXPECT_DOUBLE_EQ(centers2[6](1), (unsigned int)0);

  for (const auto i : make_range(hl16.nPins()))
    EXPECT_DOUBLE_EQ(centers2[i](2), 0.0);

  auto & centers3 = hl14.pinCenters();
  EXPECT_EQ(centers3.size(), hl14.nPins());
  EXPECT_DOUBLE_EQ(centers3[0](0), 0.0);
  EXPECT_DOUBLE_EQ(centers3[0](1), 0.0);
  EXPECT_DOUBLE_EQ(centers3[1](0), cos60 * p);
  EXPECT_DOUBLE_EQ(centers3[1](1), sin60 * p);
  EXPECT_DOUBLE_EQ(centers3[2](0), -cos60 * p);
  EXPECT_DOUBLE_EQ(centers3[2](1), sin60 * p);
  EXPECT_DOUBLE_EQ(centers3[3](0), -p);
  EXPECT_DOUBLE_EQ(centers3[3](1), 0.0);
  EXPECT_DOUBLE_EQ(centers3[4](0), -cos60 * p);
  EXPECT_DOUBLE_EQ(centers3[4](1), -sin60 * p);
  EXPECT_DOUBLE_EQ(centers3[5](0), cos60 * p);
  EXPECT_DOUBLE_EQ(centers3[5](1), -sin60 * p);
  EXPECT_DOUBLE_EQ(centers3[6](0), p);
  EXPECT_DOUBLE_EQ(centers3[6](1), (unsigned int)0);

  EXPECT_DOUBLE_EQ(centers3[7](0), p);
  EXPECT_DOUBLE_EQ(centers3[7](1), 2 * p * sin60);
  EXPECT_DOUBLE_EQ(centers3[8](0), (unsigned int)0);
  EXPECT_DOUBLE_EQ(centers3[8](1), 2 * p * sin60);
  EXPECT_DOUBLE_EQ(centers3[9](0), -p);
  EXPECT_DOUBLE_EQ(centers3[9](1), 2 * p * sin60);
  EXPECT_DOUBLE_EQ(centers3[10](0), -p - p * cos60);
  EXPECT_DOUBLE_EQ(centers3[10](1), p * sin60);
  EXPECT_DOUBLE_EQ(centers3[11](0), -2 * p);
  EXPECT_DOUBLE_EQ(centers3[11](1), (unsigned int)0);
  EXPECT_DOUBLE_EQ(centers3[12](0), -p - p * cos60);
  EXPECT_DOUBLE_EQ(centers3[12](1), -p * sin60);
  EXPECT_DOUBLE_EQ(centers3[13](0), -p);
  EXPECT_DOUBLE_EQ(centers3[13](1), -2 * p * sin60);
  EXPECT_DOUBLE_EQ(centers3[14](0), (unsigned int)0);
  EXPECT_DOUBLE_EQ(centers3[14](1), -2 * p * sin60);
  EXPECT_DOUBLE_EQ(centers3[15](0), p);
  EXPECT_DOUBLE_EQ(centers3[15](1), -2 * p * sin60);
  EXPECT_DOUBLE_EQ(centers3[16](0), p + p * cos60);
  EXPECT_DOUBLE_EQ(centers3[16](1), -p * sin60);
  EXPECT_DOUBLE_EQ(centers3[17](0), 2 * p);
  EXPECT_DOUBLE_EQ(centers3[17](1), 0.0);
  EXPECT_DOUBLE_EQ(centers3[18](0), p + p * cos60);
  EXPECT_DOUBLE_EQ(centers3[18](1), p * sin60);

  for (const auto i : make_range(hl14.nPins()))
    EXPECT_DOUBLE_EQ(centers3[i](2), 0.0);

  auto & centers4 = hl17.pinCenters();
  EXPECT_EQ(centers4.size(), hl17.nPins());
  EXPECT_DOUBLE_EQ(centers4[0](0), 0.0);
  EXPECT_DOUBLE_EQ(centers4[0](1), 0.0);
  EXPECT_DOUBLE_EQ(centers4[1](0), cos60 * p);
  EXPECT_DOUBLE_EQ(centers4[1](1), sin60 * p);
  EXPECT_DOUBLE_EQ(centers4[2](0), -cos60 * p);
  EXPECT_DOUBLE_EQ(centers4[2](1), sin60 * p);
  EXPECT_DOUBLE_EQ(centers4[3](0), -p);
  EXPECT_DOUBLE_EQ(centers4[3](1), 0.0);
  EXPECT_DOUBLE_EQ(centers4[4](0), -cos60 * p);
  EXPECT_DOUBLE_EQ(centers4[4](1), -sin60 * p);
  EXPECT_DOUBLE_EQ(centers4[5](0), cos60 * p);
  EXPECT_DOUBLE_EQ(centers4[5](1), -sin60 * p);
  EXPECT_DOUBLE_EQ(centers4[6](0), p);
  EXPECT_DOUBLE_EQ(centers4[6](1), (unsigned int)0);

  EXPECT_DOUBLE_EQ(centers4[7](0), p);
  EXPECT_DOUBLE_EQ(centers4[7](1), 2 * p * sin60);
  EXPECT_DOUBLE_EQ(centers4[8](0), (unsigned int)0);
  EXPECT_DOUBLE_EQ(centers4[8](1), 2 * p * sin60);
  EXPECT_DOUBLE_EQ(centers4[9](0), -p);
  EXPECT_DOUBLE_EQ(centers4[9](1), 2 * p * sin60);
  EXPECT_DOUBLE_EQ(centers4[10](0), -p - p * cos60);
  EXPECT_DOUBLE_EQ(centers4[10](1), p * sin60);
  EXPECT_DOUBLE_EQ(centers4[11](0), -2 * p);
  EXPECT_DOUBLE_EQ(centers4[11](1), (unsigned int)0);
  EXPECT_DOUBLE_EQ(centers4[12](0), -p - p * cos60);
  EXPECT_DOUBLE_EQ(centers4[12](1), -p * sin60);
  EXPECT_DOUBLE_EQ(centers4[13](0), -p);
  EXPECT_DOUBLE_EQ(centers4[13](1), -2 * p * sin60);
  EXPECT_DOUBLE_EQ(centers4[14](0), (unsigned int)0);
  EXPECT_DOUBLE_EQ(centers4[14](1), -2 * p * sin60);
  EXPECT_DOUBLE_EQ(centers4[15](0), p);
  EXPECT_DOUBLE_EQ(centers4[15](1), -2 * p * sin60);
  EXPECT_DOUBLE_EQ(centers4[16](0), p + p * cos60);
  EXPECT_DOUBLE_EQ(centers4[16](1), -p * sin60);
  EXPECT_DOUBLE_EQ(centers4[17](0), 2 * p);
  EXPECT_DOUBLE_EQ(centers4[17](1), 0.0);
  EXPECT_DOUBLE_EQ(centers4[18](0), p + p * cos60);
  EXPECT_DOUBLE_EQ(centers4[18](1), p * sin60);

  EXPECT_DOUBLE_EQ(centers4[19](0), p + p * cos60);
  EXPECT_DOUBLE_EQ(centers4[19](1), 3 * p * sin60);
  EXPECT_DOUBLE_EQ(centers4[20](0), p * cos60);
  EXPECT_DOUBLE_EQ(centers4[20](1), 3 * p * sin60);
  EXPECT_DOUBLE_EQ(centers4[21](0), -p * cos60);
  EXPECT_DOUBLE_EQ(centers4[21](1), 3 * p * sin60);
  EXPECT_DOUBLE_EQ(centers4[22](0), -p - p * cos60);
  EXPECT_DOUBLE_EQ(centers4[22](1), 3 * p * sin60);
  EXPECT_DOUBLE_EQ(centers4[23](0), -2 * p);
  EXPECT_DOUBLE_EQ(centers4[23](1), 2 * p * sin60);
  EXPECT_DOUBLE_EQ(centers4[24](0), -2 * p - p * cos60);
  EXPECT_DOUBLE_EQ(centers4[24](1), p * sin60);
  EXPECT_DOUBLE_EQ(centers4[25](0), -3 * p);
  EXPECT_DOUBLE_EQ(centers4[25](1), (unsigned int)0);
  EXPECT_DOUBLE_EQ(centers4[26](0), -2 * p - p * cos60);
  EXPECT_DOUBLE_EQ(centers4[26](1), -p * sin60);
  EXPECT_DOUBLE_EQ(centers4[27](0), -2 * p);
  EXPECT_DOUBLE_EQ(centers4[27](1), -2 * p * sin60);
  EXPECT_DOUBLE_EQ(centers4[28](0), -p - p * cos60);
  EXPECT_DOUBLE_EQ(centers4[28](1), -3 * p * sin60);
  EXPECT_DOUBLE_EQ(centers4[29](0), -p * cos60);
  EXPECT_DOUBLE_EQ(centers4[29](1), -3 * p * sin60);
  EXPECT_DOUBLE_EQ(centers4[30](0), p * cos60);
  EXPECT_DOUBLE_EQ(centers4[30](1), -3 * p * sin60);
  EXPECT_DOUBLE_EQ(centers4[31](0), p + p * cos60);
  EXPECT_DOUBLE_EQ(centers4[31](1), -3 * p * sin60);
  EXPECT_DOUBLE_EQ(centers4[32](0), 2 * p);
  EXPECT_DOUBLE_EQ(centers4[32](1), -2 * p * sin60);
  EXPECT_DOUBLE_EQ(centers4[33](0), 2 * p + p * cos60);
  EXPECT_DOUBLE_EQ(centers4[33](1), -p * sin60);
  EXPECT_DOUBLE_EQ(centers4[34](0), 3 * p);
  EXPECT_DOUBLE_EQ(centers4[34](1), (unsigned int)0);
  EXPECT_DOUBLE_EQ(centers4[35](0), 2 * p + p * cos60);
  EXPECT_DOUBLE_EQ(centers4[35](1), p * sin60);
  EXPECT_DOUBLE_EQ(centers4[36](0), 2 * p);
  EXPECT_DOUBLE_EQ(centers4[36](1), 2 * p * sin60);

  for (const auto i : make_range(hl17.nPins()))
    EXPECT_DOUBLE_EQ(centers4[i](2), 0.0);
}

TEST_F(HexagonalLatticeTest, channel_index_shifted)
{
  Real bp = 4.0, pp = 0.8, pd = 0.6, wd = 0.05, wp = 50.0;
  unsigned int nr = 3, a = 2;
  HexagonalLatticeUtils hl(bp, pp, pd, wd, wp, nr, a);

  // check that the z-coordinate doesn't affect the channel index identification
  Point pt0(0.06, 0.35, 3.5);
  Point pt1(-0.47, 0.28, 3.5);
  Point pt2(-0.26, -0.21, 3.5);
  Point pt3(-0.12, -0.349, 3.5);
  Point pt4(0.46, -0.27, 3.5);
  Point pt5(0.37, 0.6, 3.5);

  EXPECT_EQ(hl.channelIndex(pt0), (unsigned int)0);
  EXPECT_EQ(hl.channelIndex(pt1), (unsigned int)1);
  EXPECT_EQ(hl.channelIndex(pt2), (unsigned int)2);
  EXPECT_EQ(hl.channelIndex(pt3), (unsigned int)3);
  EXPECT_EQ(hl.channelIndex(pt4), (unsigned int)4);
  EXPECT_EQ(hl.channelIndex(pt5), (unsigned int)5);

  Point pt6(0.36, 1.06, -7.0);
  Point pt7(0.11, 0.98, -7.0);
  Point pt8(-0.43, 1.27, -7.0);
  Point pt9(-0.81, 0.93, -7.0);
  Point pt10(-0.75, 0.47, -7.0);
  Point pt11(-1.06, 0.28, -7.0);
  Point pt12(-1.16, -0.13, -7.0);
  Point pt13(-0.73, -0.41, -7.0);
  Point pt14(-0.73, -0.81, -7.0);
  Point pt15(-0.46, -1.18, -7.0);
  Point pt16(0.05, -0.98, -7.0);
  Point pt17(0.27, -1.00, -7.0);
  Point pt18(0.72, -0.98, -7.0);
  Point pt19(0.75, -0.58, -7.0);
  Point pt20(1.23, -0.23, -7.0);
  Point pt21(1.17, 0.09, -7.0);
  Point pt22(0.78, 0.38, -7.0);
  Point pt23(0.74, 0.84, -7.0);

  EXPECT_EQ(hl.channelIndex(pt6), (unsigned int)6);
  EXPECT_EQ(hl.channelIndex(pt7), (unsigned int)7);
  EXPECT_EQ(hl.channelIndex(pt8), (unsigned int)8);
  EXPECT_EQ(hl.channelIndex(pt9), (unsigned int)9);
  EXPECT_EQ(hl.channelIndex(pt10), (unsigned int)10);
  EXPECT_EQ(hl.channelIndex(pt11), (unsigned int)11);
  EXPECT_EQ(hl.channelIndex(pt12), (unsigned int)12);
  EXPECT_EQ(hl.channelIndex(pt13), (unsigned int)13);
  EXPECT_EQ(hl.channelIndex(pt14), (unsigned int)14);
  EXPECT_EQ(hl.channelIndex(pt15), (unsigned int)15);
  EXPECT_EQ(hl.channelIndex(pt16), (unsigned int)16);
  EXPECT_EQ(hl.channelIndex(pt17), (unsigned int)17);
  EXPECT_EQ(hl.channelIndex(pt18), (unsigned int)18);
  EXPECT_EQ(hl.channelIndex(pt19), (unsigned int)19);
  EXPECT_EQ(hl.channelIndex(pt20), (unsigned int)20);
  EXPECT_EQ(hl.channelIndex(pt21), (unsigned int)21);
  EXPECT_EQ(hl.channelIndex(pt22), (unsigned int)22);
  EXPECT_EQ(hl.channelIndex(pt23), (unsigned int)23);

  Point pt24(0.31, 1.44, -0.1);
  Point pt25(-0.38, 1.61, -0.1);
  Point pt26(-1.17, 1.52, -0.1);
  Point pt27(-1.78, 0.38, -0.1);
  Point pt28(-1.91, -0.42, -0.1);
  Point pt29(-1.39, -1.24, -0.1);
  Point pt30(-0.46, -1.74, -0.1);
  Point pt31(0.18, -1.79, -0.1);
  Point pt32(1.24, -1.17, -0.1);
  Point pt33(1.75, -0.57, -0.1);
  Point pt34(1.51, 0.37, -0.1);
  Point pt35(1.16, 1.42, -0.1);

  EXPECT_EQ(hl.channelIndex(pt24), (unsigned int)24);
  EXPECT_EQ(hl.channelIndex(pt25), (unsigned int)25);
  // EXPECT_EQ(hl.channelIndex(pt26), (unsigned int)26);
  EXPECT_EQ(hl.channelIndex(pt27), (unsigned int)27);
  EXPECT_EQ(hl.channelIndex(pt28), (unsigned int)28);
  EXPECT_EQ(hl.channelIndex(pt29), (unsigned int)29);
  EXPECT_EQ(hl.channelIndex(pt30), (unsigned int)30);
  EXPECT_EQ(hl.channelIndex(pt31), (unsigned int)31);
  EXPECT_EQ(hl.channelIndex(pt32), (unsigned int)32);
  EXPECT_EQ(hl.channelIndex(pt33), (unsigned int)33);
  EXPECT_EQ(hl.channelIndex(pt34), (unsigned int)34);
  EXPECT_EQ(hl.channelIndex(pt35), (unsigned int)35);

  Point pt36(1.05, 1.75, 1.2);
  Point pt37(-1.02, 1.72, 1.2);
  Point pt38(-2.03, -0.05, 1.2);
  Point pt39(-1.01, -1.59, 1.2);
  Point pt40(0.89, -1.79, 1.2);
  Point pt41(1.98, 0.12, 1.2);

  EXPECT_EQ(hl.channelIndex(pt36), (unsigned int)36);
  EXPECT_EQ(hl.channelIndex(pt37), (unsigned int)37);
  EXPECT_EQ(hl.channelIndex(pt38), (unsigned int)38);
  EXPECT_EQ(hl.channelIndex(pt39), (unsigned int)39);
  EXPECT_EQ(hl.channelIndex(pt40), (unsigned int)40);
  EXPECT_EQ(hl.channelIndex(pt41), (unsigned int)41);
}

TEST_F(HexagonalLatticeTest, channel_index)
{
  Real bp = 4.0, pp = 0.8, pd = 0.6, wd = 0.05, wp = 50.0;
  unsigned int nr = 3, a = 2;
  HexagonalLatticeUtils hl(bp, pp, pd, wd, wp, nr, a);

  Point pt0(0.06, 0.35, 0.0);
  Point pt1(-0.47, 0.28, 0.0);
  Point pt2(-0.26, -0.21, 0.0);
  Point pt3(-0.12, -0.349, 0.0);
  Point pt4(0.46, -0.27, 0.0);
  Point pt5(0.37, 0.6, 0.0);
  EXPECT_EQ(hl.channelType(pt0), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt1), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt2), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt3), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt4), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt5), channel_type::interior);
  EXPECT_EQ(hl.channelIndex(pt0), (unsigned int)0);
  EXPECT_EQ(hl.channelIndex(pt1), (unsigned int)1);
  EXPECT_EQ(hl.channelIndex(pt2), (unsigned int)2);
  EXPECT_EQ(hl.channelIndex(pt3), (unsigned int)3);
  EXPECT_EQ(hl.channelIndex(pt4), (unsigned int)4);
  EXPECT_EQ(hl.channelIndex(pt5), (unsigned int)5);

  // check that a point exactly on the edge falls into one channel
  pt0 = {0.5 * 0.8 * 0.5, 0.5 * 0.8 * std::sqrt(3.0) / 2.0, 0.0};
  EXPECT_EQ(hl.channelIndex(pt0), (unsigned int)0);

  Point pt6(0.36, 1.06, 0.0);
  Point pt7(0.11, 0.98, 0.0);
  Point pt8(-0.43, 1.27, 0.0);
  Point pt9(-0.81, 0.93, 0.0);
  Point pt10(-0.75, 0.47, 0.0);
  Point pt11(-1.06, 0.28, 0.0);
  Point pt12(-1.16, -0.13, 0.0);
  Point pt13(-0.73, -0.41, 0.0);
  Point pt14(-0.73, -0.81, 0.0);
  Point pt15(-0.46, -1.18, 0.0);
  Point pt16(0.05, -0.98, 0.0);
  Point pt17(0.27, -1.00, 0.0);
  Point pt18(0.72, -0.98, 0.0);
  Point pt19(0.75, -0.58, 0.0);
  Point pt20(1.23, -0.23, 0.0);
  Point pt21(1.17, 0.09, 0.0);
  Point pt22(0.78, 0.38, 0.0);
  Point pt23(0.74, 0.84, 0.0);

  EXPECT_EQ(hl.channelType(pt6), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt7), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt8), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt9), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt10), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt11), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt12), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt13), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt14), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt15), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt16), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt17), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt18), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt19), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt20), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt21), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt22), channel_type::interior);
  EXPECT_EQ(hl.channelType(pt23), channel_type::interior);
  EXPECT_EQ(hl.channelIndex(pt6), (unsigned int)6);
  EXPECT_EQ(hl.channelIndex(pt7), (unsigned int)7);
  EXPECT_EQ(hl.channelIndex(pt8), (unsigned int)8);
  EXPECT_EQ(hl.channelIndex(pt9), (unsigned int)9);
  EXPECT_EQ(hl.channelIndex(pt10), (unsigned int)10);
  EXPECT_EQ(hl.channelIndex(pt11), (unsigned int)11);
  EXPECT_EQ(hl.channelIndex(pt12), (unsigned int)12);
  EXPECT_EQ(hl.channelIndex(pt13), (unsigned int)13);
  EXPECT_EQ(hl.channelIndex(pt14), (unsigned int)14);
  EXPECT_EQ(hl.channelIndex(pt15), (unsigned int)15);
  EXPECT_EQ(hl.channelIndex(pt16), (unsigned int)16);
  EXPECT_EQ(hl.channelIndex(pt17), (unsigned int)17);
  EXPECT_EQ(hl.channelIndex(pt18), (unsigned int)18);
  EXPECT_EQ(hl.channelIndex(pt19), (unsigned int)19);
  EXPECT_EQ(hl.channelIndex(pt20), (unsigned int)20);
  EXPECT_EQ(hl.channelIndex(pt21), (unsigned int)21);
  EXPECT_EQ(hl.channelIndex(pt22), (unsigned int)22);
  EXPECT_EQ(hl.channelIndex(pt23), (unsigned int)23);

  Point pt24(0.31, 1.44, 0.0);
  Point pt25(-0.38, 1.61, 0.0);
  Point pt26(-1.17, 1.52, 0.0);
  Point pt27(-1.78, 0.38, 0.0);
  Point pt28(-1.91, -0.42, 0.0);
  Point pt29(-1.39, -1.24, 0.0);
  Point pt30(-0.46, -1.74, 0.0);
  Point pt31(0.18, -1.79, 0.0);
  Point pt32(1.24, -1.17, 0.0);
  Point pt33(1.75, -0.57, 0.0);
  Point pt34(1.51, 0.37, 0.0);
  Point pt35(1.16, 1.42, 0.0);

  EXPECT_EQ(hl.channelType(pt24), channel_type::edge);
  EXPECT_EQ(hl.channelType(pt25), channel_type::edge);
  EXPECT_EQ(hl.channelType(pt26), channel_type::edge);
  EXPECT_EQ(hl.channelType(pt27), channel_type::edge);
  EXPECT_EQ(hl.channelType(pt28), channel_type::edge);
  EXPECT_EQ(hl.channelType(pt29), channel_type::edge);
  EXPECT_EQ(hl.channelType(pt30), channel_type::edge);
  EXPECT_EQ(hl.channelType(pt31), channel_type::edge);
  EXPECT_EQ(hl.channelType(pt32), channel_type::edge);
  EXPECT_EQ(hl.channelType(pt33), channel_type::edge);
  EXPECT_EQ(hl.channelType(pt34), channel_type::edge);
  EXPECT_EQ(hl.channelType(pt35), channel_type::edge);
  EXPECT_EQ(hl.channelIndex(pt24), (unsigned int)24);
  EXPECT_EQ(hl.channelIndex(pt25), (unsigned int)25);
  // EXPECT_EQ(hl.channelIndex(pt26), (unsigned int)26);
  EXPECT_EQ(hl.channelIndex(pt27), (unsigned int)27);
  EXPECT_EQ(hl.channelIndex(pt28), (unsigned int)28);
  EXPECT_EQ(hl.channelIndex(pt29), (unsigned int)29);
  EXPECT_EQ(hl.channelIndex(pt30), (unsigned int)30);
  EXPECT_EQ(hl.channelIndex(pt31), (unsigned int)31);
  EXPECT_EQ(hl.channelIndex(pt32), (unsigned int)32);
  EXPECT_EQ(hl.channelIndex(pt33), (unsigned int)33);
  EXPECT_EQ(hl.channelIndex(pt34), (unsigned int)34);
  EXPECT_EQ(hl.channelIndex(pt35), (unsigned int)35);

  Point pt36(1.05, 1.75, 0.0);
  Point pt37(-1.02, 1.72, 0.0);
  Point pt38(-2.03, -0.05, 0.0);
  Point pt39(-1.01, -1.59, 0.0);
  Point pt40(0.89, -1.79, 0.0);
  Point pt41(1.98, 0.12, 0.0);

  EXPECT_EQ(hl.channelType(pt36), channel_type::corner);
  EXPECT_EQ(hl.channelType(pt37), channel_type::corner);
  EXPECT_EQ(hl.channelType(pt38), channel_type::corner);
  EXPECT_EQ(hl.channelType(pt39), channel_type::corner);
  EXPECT_EQ(hl.channelType(pt40), channel_type::corner);
  EXPECT_EQ(hl.channelType(pt41), channel_type::corner);
  EXPECT_EQ(hl.channelIndex(pt36), (unsigned int)36);
  EXPECT_EQ(hl.channelIndex(pt37), (unsigned int)37);
  EXPECT_EQ(hl.channelIndex(pt38), (unsigned int)38);
  EXPECT_EQ(hl.channelIndex(pt39), (unsigned int)39);
  EXPECT_EQ(hl.channelIndex(pt40), (unsigned int)40);
  EXPECT_EQ(hl.channelIndex(pt41), (unsigned int)41);
}

TEST_F(HexagonalLatticeTest, gaps1)
{
  Real bp = 4.0, pp = 0.8, pd = 0.6, wd = 0.05, wp = 50.0;
  unsigned int nr = 1, a = 2;
  HexagonalLatticeUtils hl(bp, pp, pd, wd, wp, nr, a);
  const auto & gi = hl.gapIndices();
  const auto & lg = hl.localToGlobalGaps();

  EXPECT_EQ(hl.nInteriorGaps(), (unsigned int)0);

  int i = 0;
  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, -1);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, -2);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, -3);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, -4);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, -5);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, -6);

  for (unsigned int i = 0; i < hl.nGaps(); ++i)
    EXPECT_FALSE(hl.lastGapInRing(i));

  i = 0;
  EXPECT_EQ(lg[i][0], 5);
  EXPECT_EQ(lg[i++][1], 0);

  EXPECT_EQ(lg[i][0], 0);
  EXPECT_EQ(lg[i++][1], 1);

  EXPECT_EQ(lg[i][0], 1);
  EXPECT_EQ(lg[i++][1], 2);

  EXPECT_EQ(lg[i][0], 2);
  EXPECT_EQ(lg[i++][1], 3);

  EXPECT_EQ(lg[i][0], 3);
  EXPECT_EQ(lg[i++][1], 4);

  EXPECT_EQ(lg[i][0], 4);
  EXPECT_EQ(lg[i++][1], 5);
}

TEST_F(HexagonalLatticeTest, gaps2)
{
  Real bp = 4.0, pp = 0.8, pd = 0.6, wd = 0.05, wp = 50.0;
  unsigned int nr = 2, a = 2;
  HexagonalLatticeUtils hl(bp, pp, pd, wd, wp, nr, a);
  const auto & gi = hl.gapIndices();
  const auto & lg = hl.localToGlobalGaps();

  EXPECT_EQ(hl.nInteriorGaps(), (unsigned int)12);

  int i = 0;
  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 1);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 2);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 3);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 4);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 5);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 6);

  EXPECT_EQ(gi[i].first, 1);
  EXPECT_EQ(gi[i++].second, 2);

  EXPECT_EQ(gi[i].first, 1);
  EXPECT_EQ(gi[i++].second, 6);

  EXPECT_EQ(gi[i].first, 2);
  EXPECT_EQ(gi[i++].second, 3);

  EXPECT_EQ(gi[i].first, 3);
  EXPECT_EQ(gi[i++].second, 4);

  EXPECT_EQ(gi[i].first, 4);
  EXPECT_EQ(gi[i++].second, 5);

  EXPECT_EQ(gi[i].first, 5);
  EXPECT_EQ(gi[i++].second, 6);

  EXPECT_EQ(gi[i].first, 1);
  EXPECT_EQ(gi[i++].second, -1);

  EXPECT_EQ(gi[i].first, 2);
  EXPECT_EQ(gi[i++].second, -1);

  EXPECT_EQ(gi[i].first, 2);
  EXPECT_EQ(gi[i++].second, -2);

  EXPECT_EQ(gi[i].first, 3);
  EXPECT_EQ(gi[i++].second, -2);

  EXPECT_EQ(gi[i].first, 3);
  EXPECT_EQ(gi[i++].second, -3);

  EXPECT_EQ(gi[i].first, 4);
  EXPECT_EQ(gi[i++].second, -3);

  EXPECT_EQ(gi[i].first, 4);
  EXPECT_EQ(gi[i++].second, -4);

  EXPECT_EQ(gi[i].first, 5);
  EXPECT_EQ(gi[i++].second, -4);

  EXPECT_EQ(gi[i].first, 5);
  EXPECT_EQ(gi[i++].second, -5);

  EXPECT_EQ(gi[i].first, 6);
  EXPECT_EQ(gi[i++].second, -5);

  EXPECT_EQ(gi[i].first, 6);
  EXPECT_EQ(gi[i++].second, -6);

  EXPECT_EQ(gi[i].first, 1);
  EXPECT_EQ(gi[i++].second, -6);

  for (unsigned int i = 0; i < hl.nGaps(); ++i)
  {
    if (i == 7)
      EXPECT_TRUE(hl.lastGapInRing(i));
    else
      EXPECT_FALSE(hl.lastGapInRing(i));
  }

  i = 0;
  EXPECT_EQ(lg[i][0], 0);
  EXPECT_EQ(lg[i][1], 6);
  EXPECT_EQ(lg[i++][2], 1);

  EXPECT_EQ(lg[i][0], 1);
  EXPECT_EQ(lg[i][1], 8);
  EXPECT_EQ(lg[i++][2], 2);

  EXPECT_EQ(lg[i][0], 2);
  EXPECT_EQ(lg[i][1], 9);
  EXPECT_EQ(lg[i++][2], 3);

  EXPECT_EQ(lg[i][0], 3);
  EXPECT_EQ(lg[i][1], 10);
  EXPECT_EQ(lg[i++][2], 4);

  EXPECT_EQ(lg[i][0], 4);
  EXPECT_EQ(lg[i][1], 11);
  EXPECT_EQ(lg[i++][2], 5);

  EXPECT_EQ(lg[i][0], 5);
  EXPECT_EQ(lg[i][1], 7);
  EXPECT_EQ(lg[i++][2], 0);

  EXPECT_EQ(lg[i][0], 6);
  EXPECT_EQ(lg[i][1], 13);
  EXPECT_EQ(lg[i++][2], 12);

  EXPECT_EQ(lg[i][0], 8);
  EXPECT_EQ(lg[i][1], 15);
  EXPECT_EQ(lg[i++][2], 14);

  EXPECT_EQ(lg[i][0], 9);
  EXPECT_EQ(lg[i][1], 17);
  EXPECT_EQ(lg[i++][2], 16);

  EXPECT_EQ(lg[i][0], 10);
  EXPECT_EQ(lg[i][1], 19);
  EXPECT_EQ(lg[i++][2], 18);

  EXPECT_EQ(lg[i][0], 11);
  EXPECT_EQ(lg[i][1], 21);
  EXPECT_EQ(lg[i++][2], 20);

  EXPECT_EQ(lg[i][0], 7);
  EXPECT_EQ(lg[i][1], 23);
  EXPECT_EQ(lg[i++][2], 22);

  EXPECT_EQ(lg[i][0], 23);
  EXPECT_EQ(lg[i++][1], 12);

  EXPECT_EQ(lg[i][0], 13);
  EXPECT_EQ(lg[i++][1], 14);

  EXPECT_EQ(lg[i][0], 15);
  EXPECT_EQ(lg[i++][1], 16);

  EXPECT_EQ(lg[i][0], 17);
  EXPECT_EQ(lg[i++][1], 18);

  EXPECT_EQ(lg[i][0], 19);
  EXPECT_EQ(lg[i++][1], 20);

  EXPECT_EQ(lg[i][0], 21);
  EXPECT_EQ(lg[i++][1], 22);
}

TEST_F(HexagonalLatticeTest, gaps3)
{
  Real bp = 4.0, pp = 0.8, pd = 0.6, wd = 0.05, wp = 50.0;
  unsigned int nr = 3, a = 2;
  HexagonalLatticeUtils hl(bp, pp, pd, wd, wp, nr, a);
  const auto & gi = hl.gapIndices();
  const auto & lg = hl.localToGlobalGaps();

  EXPECT_EQ(hl.nInteriorGaps(), (unsigned int)42);

  int i = 0;
  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 1);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 2);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 3);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 4);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 5);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 6);

  EXPECT_EQ(gi[i].first, 1);
  EXPECT_EQ(gi[i++].second, 2);

  EXPECT_EQ(gi[i].first, 1);
  EXPECT_EQ(gi[i++].second, 6);

  EXPECT_EQ(gi[i].first, 1);
  EXPECT_EQ(gi[i++].second, 7);

  EXPECT_EQ(gi[i].first, 1);
  EXPECT_EQ(gi[i++].second, 8);

  EXPECT_EQ(gi[i].first, 1);
  EXPECT_EQ(gi[i++].second, 18);

  EXPECT_EQ(gi[i].first, 2);
  EXPECT_EQ(gi[i++].second, 3);

  EXPECT_EQ(gi[i].first, 2);
  EXPECT_EQ(gi[i++].second, 8);

  EXPECT_EQ(gi[i].first, 2);
  EXPECT_EQ(gi[i++].second, 9);

  EXPECT_EQ(gi[i].first, 2);
  EXPECT_EQ(gi[i++].second, 10);

  EXPECT_EQ(gi[i].first, 3);
  EXPECT_EQ(gi[i++].second, 4);

  EXPECT_EQ(gi[i].first, 3);
  EXPECT_EQ(gi[i++].second, 10);

  EXPECT_EQ(gi[i].first, 3);
  EXPECT_EQ(gi[i++].second, 11);

  EXPECT_EQ(gi[i].first, 3);
  EXPECT_EQ(gi[i++].second, 12);

  EXPECT_EQ(gi[i].first, 4);
  EXPECT_EQ(gi[i++].second, 5);

  EXPECT_EQ(gi[i].first, 4);
  EXPECT_EQ(gi[i++].second, 12);

  EXPECT_EQ(gi[i].first, 4);
  EXPECT_EQ(gi[i++].second, 13);

  EXPECT_EQ(gi[i].first, 4);
  EXPECT_EQ(gi[i++].second, 14);

  EXPECT_EQ(gi[i].first, 5);
  EXPECT_EQ(gi[i++].second, 6);

  EXPECT_EQ(gi[i].first, 5);
  EXPECT_EQ(gi[i++].second, 14);

  EXPECT_EQ(gi[i].first, 5);
  EXPECT_EQ(gi[i++].second, 15);

  EXPECT_EQ(gi[i].first, 5);
  EXPECT_EQ(gi[i++].second, 16);

  EXPECT_EQ(gi[i].first, 6);
  EXPECT_EQ(gi[i++].second, 16);

  EXPECT_EQ(gi[i].first, 6);
  EXPECT_EQ(gi[i++].second, 17);

  EXPECT_EQ(gi[i].first, 6);
  EXPECT_EQ(gi[i++].second, 18);

  EXPECT_EQ(gi[i].first, 7);
  EXPECT_EQ(gi[i++].second, 8);

  EXPECT_EQ(gi[i].first, 7);
  EXPECT_EQ(gi[i++].second, 18);

  EXPECT_EQ(gi[i].first, 8);
  EXPECT_EQ(gi[i++].second, 9);

  EXPECT_EQ(gi[i].first, 9);
  EXPECT_EQ(gi[i++].second, 10);

  EXPECT_EQ(gi[i].first, 10);
  EXPECT_EQ(gi[i++].second, 11);

  EXPECT_EQ(gi[i].first, 11);
  EXPECT_EQ(gi[i++].second, 12);

  EXPECT_EQ(gi[i].first, 12);
  EXPECT_EQ(gi[i++].second, 13);

  EXPECT_EQ(gi[i].first, 13);
  EXPECT_EQ(gi[i++].second, 14);

  EXPECT_EQ(gi[i].first, 14);
  EXPECT_EQ(gi[i++].second, 15);

  EXPECT_EQ(gi[i].first, 15);
  EXPECT_EQ(gi[i++].second, 16);

  EXPECT_EQ(gi[i].first, 16);
  EXPECT_EQ(gi[i++].second, 17);

  EXPECT_EQ(gi[i].first, 17);
  EXPECT_EQ(gi[i++].second, 18);

  EXPECT_EQ(gi[i].first, 7);
  EXPECT_EQ(gi[i++].second, -1);

  EXPECT_EQ(gi[i].first, 8);
  EXPECT_EQ(gi[i++].second, -1);

  EXPECT_EQ(gi[i].first, 9);
  EXPECT_EQ(gi[i++].second, -1);

  EXPECT_EQ(gi[i].first, 9);
  EXPECT_EQ(gi[i++].second, -2);

  EXPECT_EQ(gi[i].first, 10);
  EXPECT_EQ(gi[i++].second, -2);

  EXPECT_EQ(gi[i].first, 11);
  EXPECT_EQ(gi[i++].second, -2);

  EXPECT_EQ(gi[i].first, 11);
  EXPECT_EQ(gi[i++].second, -3);

  EXPECT_EQ(gi[i].first, 12);
  EXPECT_EQ(gi[i++].second, -3);

  EXPECT_EQ(gi[i].first, 13);
  EXPECT_EQ(gi[i++].second, -3);

  EXPECT_EQ(gi[i].first, 13);
  EXPECT_EQ(gi[i++].second, -4);

  EXPECT_EQ(gi[i].first, 14);
  EXPECT_EQ(gi[i++].second, -4);

  EXPECT_EQ(gi[i].first, 15);
  EXPECT_EQ(gi[i++].second, -4);

  EXPECT_EQ(gi[i].first, 15);
  EXPECT_EQ(gi[i++].second, -5);

  EXPECT_EQ(gi[i].first, 16);
  EXPECT_EQ(gi[i++].second, -5);

  EXPECT_EQ(gi[i].first, 17);
  EXPECT_EQ(gi[i++].second, -5);

  EXPECT_EQ(gi[i].first, 17);
  EXPECT_EQ(gi[i++].second, -6);

  EXPECT_EQ(gi[i].first, 18);
  EXPECT_EQ(gi[i++].second, -6);

  EXPECT_EQ(gi[i].first, 7);
  EXPECT_EQ(gi[i++].second, -6);

  for (unsigned int i = 0; i < hl.nGaps(); ++i)
  {
    if (i == 7 || i == 31)
      EXPECT_TRUE(hl.lastGapInRing(i));
    else
      EXPECT_FALSE(hl.lastGapInRing(i));
  }

  i = 0;
  EXPECT_EQ(lg[i][0], 0);
  EXPECT_EQ(lg[i][1], 6);
  EXPECT_EQ(lg[i++][2], 1);

  EXPECT_EQ(lg[i][0], 1);
  EXPECT_EQ(lg[i][1], 11);
  EXPECT_EQ(lg[i++][2], 2);

  EXPECT_EQ(lg[i][0], 2);
  EXPECT_EQ(lg[i][1], 15);
  EXPECT_EQ(lg[i++][2], 3);

  EXPECT_EQ(lg[i][0], 3);
  EXPECT_EQ(lg[i][1], 19);
  EXPECT_EQ(lg[i++][2], 4);

  EXPECT_EQ(lg[i][0], 4);
  EXPECT_EQ(lg[i][1], 23);
  EXPECT_EQ(lg[i++][2], 5);

  EXPECT_EQ(lg[i][0], 5);
  EXPECT_EQ(lg[i][1], 7);
  EXPECT_EQ(lg[i++][2], 0);

  EXPECT_EQ(lg[i][0], 8);
  EXPECT_EQ(lg[i][1], 30);
  EXPECT_EQ(lg[i++][2], 9);

  EXPECT_EQ(lg[i][0], 12);
  EXPECT_EQ(lg[i][1], 6);
  EXPECT_EQ(lg[i++][2], 9);

  EXPECT_EQ(lg[i][0], 12);
  EXPECT_EQ(lg[i][1], 32);
  EXPECT_EQ(lg[i++][2], 13);

  EXPECT_EQ(lg[i][0], 13);
  EXPECT_EQ(lg[i][1], 33);
  EXPECT_EQ(lg[i++][2], 14);

  EXPECT_EQ(lg[i][0], 16);
  EXPECT_EQ(lg[i][1], 11);
  EXPECT_EQ(lg[i++][2], 14);

  EXPECT_EQ(lg[i][0], 16);
  EXPECT_EQ(lg[i][1], 34);
  EXPECT_EQ(lg[i++][2], 17);

  EXPECT_EQ(lg[i][0], 17);
  EXPECT_EQ(lg[i][1], 35);
  EXPECT_EQ(lg[i++][2], 18);

  EXPECT_EQ(lg[i][0], 20);
  EXPECT_EQ(lg[i][1], 15);
  EXPECT_EQ(lg[i++][2], 18);

  EXPECT_EQ(lg[i][0], 20);
  EXPECT_EQ(lg[i][1], 36);
  EXPECT_EQ(lg[i++][2], 21);

  EXPECT_EQ(lg[i][0], 21);
  EXPECT_EQ(lg[i][1], 37);
  EXPECT_EQ(lg[i++][2], 22);

  EXPECT_EQ(lg[i][0], 24);
  EXPECT_EQ(lg[i][1], 19);
  EXPECT_EQ(lg[i++][2], 22);

  EXPECT_EQ(lg[i][0], 24);
  EXPECT_EQ(lg[i][1], 38);
  EXPECT_EQ(lg[i++][2], 25);

  EXPECT_EQ(lg[i][0], 25);
  EXPECT_EQ(lg[i][1], 39);
  EXPECT_EQ(lg[i++][2], 26);

  EXPECT_EQ(lg[i][0], 27);
  EXPECT_EQ(lg[i][1], 23);
  EXPECT_EQ(lg[i++][2], 26);

  EXPECT_EQ(lg[i][0], 27);
  EXPECT_EQ(lg[i][1], 40);
  EXPECT_EQ(lg[i++][2], 28);

  EXPECT_EQ(lg[i][0], 28);
  EXPECT_EQ(lg[i][1], 41);
  EXPECT_EQ(lg[i++][2], 29);

  EXPECT_EQ(lg[i][0], 10);
  EXPECT_EQ(lg[i][1], 7);
  EXPECT_EQ(lg[i++][2], 29);

  EXPECT_EQ(lg[i][0], 10);
  EXPECT_EQ(lg[i][1], 31);
  EXPECT_EQ(lg[i++][2], 8);

  // edge channels
  EXPECT_EQ(lg[i][0], 30);
  EXPECT_EQ(lg[i][1], 43);
  EXPECT_EQ(lg[i++][2], 42);

  EXPECT_EQ(lg[i][0], 32);
  EXPECT_EQ(lg[i][1], 44);
  EXPECT_EQ(lg[i++][2], 43);

  EXPECT_EQ(lg[i][0], 33);
  EXPECT_EQ(lg[i][1], 46);
  EXPECT_EQ(lg[i++][2], 45);

  EXPECT_EQ(lg[i][0], 34);
  EXPECT_EQ(lg[i][1], 47);
  EXPECT_EQ(lg[i++][2], 46);

  EXPECT_EQ(lg[i][0], 35);
  EXPECT_EQ(lg[i][1], 49);
  EXPECT_EQ(lg[i++][2], 48);

  EXPECT_EQ(lg[i][0], 36);
  EXPECT_EQ(lg[i][1], 50);
  EXPECT_EQ(lg[i++][2], 49);

  EXPECT_EQ(lg[i][0], 37);
  EXPECT_EQ(lg[i][1], 52);
  EXPECT_EQ(lg[i++][2], 51);

  EXPECT_EQ(lg[i][0], 38);
  EXPECT_EQ(lg[i][1], 53);
  EXPECT_EQ(lg[i++][2], 52);

  EXPECT_EQ(lg[i][0], 39);
  EXPECT_EQ(lg[i][1], 55);
  EXPECT_EQ(lg[i++][2], 54);

  EXPECT_EQ(lg[i][0], 40);
  EXPECT_EQ(lg[i][1], 56);
  EXPECT_EQ(lg[i++][2], 55);

  EXPECT_EQ(lg[i][0], 41);
  EXPECT_EQ(lg[i][1], 58);
  EXPECT_EQ(lg[i++][2], 57);

  EXPECT_EQ(lg[i][0], 31);
  EXPECT_EQ(lg[i][1], 59);
  EXPECT_EQ(lg[i++][2], 58);

  // corner channels
  EXPECT_EQ(lg[i][0], 59);
  EXPECT_EQ(lg[i++][1], 42);

  EXPECT_EQ(lg[i][0], 44);
  EXPECT_EQ(lg[i++][1], 45);

  EXPECT_EQ(lg[i][0], 47);
  EXPECT_EQ(lg[i++][1], 48);

  EXPECT_EQ(lg[i][0], 50);
  EXPECT_EQ(lg[i++][1], 51);

  EXPECT_EQ(lg[i][0], 53);
  EXPECT_EQ(lg[i++][1], 54);

  EXPECT_EQ(lg[i][0], 56);
  EXPECT_EQ(lg[i++][1], 57);
}

TEST_F(HexagonalLatticeTest, closest_gap)
{
  Real bp = 4.0, pp = 0.8, pd = 0.6, wd = 0.05, wp = 50.0;
  unsigned int nr = 3, a = 2;
  HexagonalLatticeUtils hl(bp, pp, pd, wd, wp, nr, a);

  Point pt1(0.23, 0.27, 0.0);
  EXPECT_EQ(hl.gapIndex(pt1), (unsigned int)0);

  Point pt2(-0.5, 0.29, 0.0);
  EXPECT_EQ(hl.gapIndex(pt2), (unsigned int)11);

  Point pt3(1.14, 0.275, 0.0);
  EXPECT_EQ(hl.gapIndex(pt3), (unsigned int)29);

  Point pt4(-0.77, 1.015, 0.0);
  EXPECT_EQ(hl.gapIndex(pt4), (unsigned int)13);

  Point pt5(-0.84, 0.445, 0.0);
  EXPECT_EQ(hl.gapIndex(pt5), (unsigned int)16);

  Point pt6(-0.47, 1.55, 0.0);
  EXPECT_EQ(hl.gapIndex(pt6), (unsigned int)32);

  Point pt7(-0.069, 1.94, 0.0);
  EXPECT_EQ(hl.gapIndex(pt7), (unsigned int)43);

  Point pt8(-1.22, 1.79, 0.0);
  EXPECT_EQ(hl.gapIndex(pt8), (unsigned int)45);

  Point pt10(-0.26, -1.61, 0.0);
  EXPECT_EQ(hl.gapIndex(pt10), (unsigned int)37);

  Point pt11(2.23, 0.03, 0.0);
  EXPECT_EQ(hl.gapIndex(pt11), (unsigned int)57);

  Point pt12(-1.77, -0.70, 0.0);
  EXPECT_EQ(hl.gapIndex(pt12), (unsigned int)49);
}

TEST_F(HexagonalLatticeTest, normals1)
{
  Real bp = 4.0, pp = 0.8, pd = 0.6, wd = 0.05, wp = 50.0;
  unsigned int nr = 1, a = 2;
  HexagonalLatticeUtils hl(bp, pp, pd, wd, wp, nr, a);
  const auto & normals = hl.gapUnitNormals();
  Real sin60 = std::sqrt(3.0) / 2.0;

  for (const auto & n : normals)
    EXPECT_DOUBLE_EQ(n(2), 0.0);

  EXPECT_DOUBLE_EQ(normals[0](0), -1.0);
  EXPECT_DOUBLE_EQ(normals[0](1), 0.0);

  EXPECT_DOUBLE_EQ(normals[1](0), -0.5);
  EXPECT_DOUBLE_EQ(normals[1](1), -sin60);

  EXPECT_DOUBLE_EQ(normals[2](0), 0.5);
  EXPECT_DOUBLE_EQ(normals[2](1), -sin60);

  EXPECT_DOUBLE_EQ(normals[3](0), 1.0);
  EXPECT_DOUBLE_EQ(normals[3](1), 0.0);

  EXPECT_DOUBLE_EQ(normals[4](0), 0.5);
  EXPECT_DOUBLE_EQ(normals[4](1), sin60);

  EXPECT_DOUBLE_EQ(normals[5](0), -0.5);
  EXPECT_DOUBLE_EQ(normals[5](1), sin60);
}

TEST_F(HexagonalLatticeTest, normals2)
{
  Real bp = 4.0, pp = 0.8, pd = 0.6, wd = 0.05, wp = 50.0;
  unsigned int nr = 2, a = 2;
  HexagonalLatticeUtils hl(bp, pp, pd, wd, wp, nr, a);
  const auto & normals = hl.gapUnitNormals();
  Real sin60 = std::sqrt(3.0) / 2.0;

  for (const auto & n : normals)
    EXPECT_DOUBLE_EQ(n(2), 0.0);

  EXPECT_DOUBLE_EQ(normals[0](0), -sin60);
  EXPECT_DOUBLE_EQ(normals[0](1), 0.5);

  EXPECT_DOUBLE_EQ(normals[1](0), -sin60);
  EXPECT_DOUBLE_EQ(normals[1](1), -0.5);

  EXPECT_DOUBLE_EQ(normals[2](0), 0.0);
  EXPECT_DOUBLE_EQ(normals[2](1), -1.0);

  EXPECT_DOUBLE_EQ(normals[3](0), sin60);
  EXPECT_DOUBLE_EQ(normals[3](1), -0.5);

  EXPECT_DOUBLE_EQ(normals[4](0), sin60);
  EXPECT_DOUBLE_EQ(normals[4](1), 0.5);

  EXPECT_DOUBLE_EQ(normals[5](0), 0.0);
  EXPECT_DOUBLE_EQ(normals[5](1), 1.0);

  EXPECT_DOUBLE_EQ(normals[6](0), 0.0);
  EXPECT_DOUBLE_EQ(normals[6](1), -1.0);

  EXPECT_DOUBLE_EQ(normals[7](0), -sin60);
  EXPECT_DOUBLE_EQ(normals[7](1), -0.5);

  EXPECT_DOUBLE_EQ(normals[8](0), sin60);
  EXPECT_DOUBLE_EQ(normals[8](1), -0.5);

  EXPECT_DOUBLE_EQ(normals[9](0), sin60);
  EXPECT_DOUBLE_EQ(normals[9](1), 0.5);

  EXPECT_DOUBLE_EQ(normals[10](0), 0.0);
  EXPECT_DOUBLE_EQ(normals[10](1), 1.0);

  EXPECT_DOUBLE_EQ(normals[11](0), -sin60);
  EXPECT_DOUBLE_EQ(normals[11](1), 0.5);

  EXPECT_DOUBLE_EQ(normals[12](0), -1.0);
  EXPECT_DOUBLE_EQ(normals[12](1), 0.0);

  EXPECT_DOUBLE_EQ(normals[13](0), -1.0);
  EXPECT_DOUBLE_EQ(normals[13](1), 0.0);

  EXPECT_DOUBLE_EQ(normals[14](0), -0.5);
  EXPECT_DOUBLE_EQ(normals[14](1), -sin60);

  EXPECT_DOUBLE_EQ(normals[15](0), -0.5);
  EXPECT_DOUBLE_EQ(normals[15](1), -sin60);

  EXPECT_DOUBLE_EQ(normals[16](0), 0.5);
  EXPECT_DOUBLE_EQ(normals[16](1), -sin60);

  EXPECT_DOUBLE_EQ(normals[17](0), 0.5);
  EXPECT_DOUBLE_EQ(normals[17](1), -sin60);

  EXPECT_DOUBLE_EQ(normals[18](0), 1.0);
  EXPECT_DOUBLE_EQ(normals[18](1), 0.0);

  EXPECT_DOUBLE_EQ(normals[19](0), 1.0);
  EXPECT_DOUBLE_EQ(normals[19](1), 0.0);

  EXPECT_DOUBLE_EQ(normals[20](0), 0.5);
  EXPECT_DOUBLE_EQ(normals[20](1), sin60);

  EXPECT_DOUBLE_EQ(normals[21](0), 0.5);
  EXPECT_DOUBLE_EQ(normals[21](1), sin60);

  EXPECT_DOUBLE_EQ(normals[22](0), -0.5);
  EXPECT_DOUBLE_EQ(normals[22](1), sin60);

  EXPECT_DOUBLE_EQ(normals[23](0), -0.5);
  EXPECT_DOUBLE_EQ(normals[23](1), sin60);
}

TEST_F(HexagonalLatticeTest, normals3)
{
  Real bp = 4.0, pp = 0.8, pd = 0.6, wd = 0.05, wp = 50.0;
  unsigned int nr = 3, a = 2;
  HexagonalLatticeUtils hl(bp, pp, pd, wd, wp, nr, a);
  const auto & normals = hl.gapUnitNormals();
  Real sin60 = std::sqrt(3.0) / 2.0;

  for (const auto & n : normals)
    EXPECT_DOUBLE_EQ(n(2), 0.0);

  EXPECT_DOUBLE_EQ(normals[0](0), -sin60);
  EXPECT_DOUBLE_EQ(normals[0](1), 0.5);

  EXPECT_DOUBLE_EQ(normals[1](0), -sin60);
  EXPECT_DOUBLE_EQ(normals[1](1), -0.5);

  EXPECT_DOUBLE_EQ(normals[2](0), 0.0);
  EXPECT_DOUBLE_EQ(normals[2](1), -1.0);

  EXPECT_DOUBLE_EQ(normals[3](0), sin60);
  EXPECT_DOUBLE_EQ(normals[3](1), -0.5);

  EXPECT_DOUBLE_EQ(normals[4](0), sin60);
  EXPECT_DOUBLE_EQ(normals[4](1), 0.5);

  EXPECT_DOUBLE_EQ(normals[5](0), 0.0);
  EXPECT_DOUBLE_EQ(normals[5](1), 1.0);

  EXPECT_DOUBLE_EQ(normals[6](0), 0.0);
  EXPECT_DOUBLE_EQ(normals[6](1), -1.0);

  EXPECT_DOUBLE_EQ(normals[7](0), -sin60);
  EXPECT_DOUBLE_EQ(normals[7](1), -0.5);

  EXPECT_DOUBLE_EQ(normals[8](0), -sin60);
  EXPECT_DOUBLE_EQ(normals[8](1), 0.5);

  EXPECT_DOUBLE_EQ(normals[9](0), -sin60);
  EXPECT_DOUBLE_EQ(normals[9](1), -0.5);

  EXPECT_DOUBLE_EQ(normals[10](0), 0.0);
  EXPECT_DOUBLE_EQ(normals[10](1), 1.0);

  EXPECT_DOUBLE_EQ(normals[11](0), sin60);
  EXPECT_DOUBLE_EQ(normals[11](1), -0.5);

  EXPECT_DOUBLE_EQ(normals[12](0), -sin60);
  EXPECT_DOUBLE_EQ(normals[12](1), 0.5);

  EXPECT_DOUBLE_EQ(normals[13](0), -sin60);
  EXPECT_DOUBLE_EQ(normals[13](1), -0.5);

  EXPECT_DOUBLE_EQ(normals[14](0), 0.0);
  EXPECT_DOUBLE_EQ(normals[14](1), -1.0);

  EXPECT_DOUBLE_EQ(normals[15](0), sin60);
  EXPECT_DOUBLE_EQ(normals[15](1), 0.5);

  EXPECT_DOUBLE_EQ(normals[16](0), -sin60);
  EXPECT_DOUBLE_EQ(normals[16](1), -0.5);

  EXPECT_DOUBLE_EQ(normals[17](0), 0.0);
  EXPECT_DOUBLE_EQ(normals[17](1), -1.0);

  EXPECT_DOUBLE_EQ(normals[18](0), sin60);
  EXPECT_DOUBLE_EQ(normals[18](1), -0.5);

  EXPECT_DOUBLE_EQ(normals[19](0), 0.0);
  EXPECT_DOUBLE_EQ(normals[19](1), 1.0);

  EXPECT_DOUBLE_EQ(normals[20](0), 0.0);
  EXPECT_DOUBLE_EQ(normals[20](1), -1.0);

  EXPECT_DOUBLE_EQ(normals[21](0), sin60);
  EXPECT_DOUBLE_EQ(normals[21](1), -0.5);

  EXPECT_DOUBLE_EQ(normals[22](0), sin60);
  EXPECT_DOUBLE_EQ(normals[22](1), 0.5);

  EXPECT_DOUBLE_EQ(normals[23](0), -sin60);
  EXPECT_DOUBLE_EQ(normals[23](1), 0.5);

  EXPECT_DOUBLE_EQ(normals[24](0), sin60);
  EXPECT_DOUBLE_EQ(normals[24](1), -0.5);

  EXPECT_DOUBLE_EQ(normals[25](0), sin60);
  EXPECT_DOUBLE_EQ(normals[25](1), 0.5);

  EXPECT_DOUBLE_EQ(normals[26](0), 0.0);
  EXPECT_DOUBLE_EQ(normals[26](1), 1.0);

  EXPECT_DOUBLE_EQ(normals[27](0), sin60);
  EXPECT_DOUBLE_EQ(normals[27](1), 0.5);

  EXPECT_DOUBLE_EQ(normals[28](0), 0.0);
  EXPECT_DOUBLE_EQ(normals[28](1), 1.0);

  EXPECT_DOUBLE_EQ(normals[29](0), -sin60);
  EXPECT_DOUBLE_EQ(normals[29](1), 0.5);

  EXPECT_DOUBLE_EQ(normals[30](0), 0.0);
  EXPECT_DOUBLE_EQ(normals[30](1), -1.0);

  EXPECT_DOUBLE_EQ(normals[31](0), -sin60);
  EXPECT_DOUBLE_EQ(normals[31](1), -0.5);

  EXPECT_DOUBLE_EQ(normals[32](0), 0.0);
  EXPECT_DOUBLE_EQ(normals[32](1), -1.0);

  EXPECT_DOUBLE_EQ(normals[33](0), sin60);
  EXPECT_DOUBLE_EQ(normals[33](1), -0.5);

  EXPECT_DOUBLE_EQ(normals[34](0), sin60);
  EXPECT_DOUBLE_EQ(normals[34](1), -0.5);

  EXPECT_DOUBLE_EQ(normals[35](0), sin60);
  EXPECT_DOUBLE_EQ(normals[35](1), 0.5);

  EXPECT_DOUBLE_EQ(normals[36](0), sin60);
  EXPECT_DOUBLE_EQ(normals[36](1), 0.5);

  EXPECT_DOUBLE_EQ(normals[37](0), 0.0);
  EXPECT_DOUBLE_EQ(normals[37](1), 1.0);

  EXPECT_DOUBLE_EQ(normals[38](0), 0.0);
  EXPECT_DOUBLE_EQ(normals[38](1), 1.0);

  EXPECT_DOUBLE_EQ(normals[39](0), -sin60);
  EXPECT_DOUBLE_EQ(normals[39](1), 0.5);

  EXPECT_DOUBLE_EQ(normals[40](0), -sin60);
  EXPECT_DOUBLE_EQ(normals[40](1), 0.5);

  EXPECT_DOUBLE_EQ(normals[41](0), -sin60);
  EXPECT_DOUBLE_EQ(normals[41](1), -0.5);

  EXPECT_DOUBLE_EQ(normals[42](0), -1.0);
  EXPECT_DOUBLE_EQ(normals[42](1), 0.0);

  EXPECT_DOUBLE_EQ(normals[43](0), -1.0);
  EXPECT_DOUBLE_EQ(normals[43](1), 0.0);

  EXPECT_DOUBLE_EQ(normals[44](0), -1.0);
  EXPECT_DOUBLE_EQ(normals[44](1), 0.0);

  EXPECT_DOUBLE_EQ(normals[45](0), -0.5);
  EXPECT_DOUBLE_EQ(normals[45](1), -sin60);

  EXPECT_DOUBLE_EQ(normals[46](0), -0.5);
  EXPECT_DOUBLE_EQ(normals[46](1), -sin60);

  EXPECT_DOUBLE_EQ(normals[47](0), -0.5);
  EXPECT_DOUBLE_EQ(normals[47](1), -sin60);

  EXPECT_DOUBLE_EQ(normals[48](0), 0.5);
  EXPECT_DOUBLE_EQ(normals[48](1), -sin60);

  EXPECT_DOUBLE_EQ(normals[49](0), 0.5);
  EXPECT_DOUBLE_EQ(normals[49](1), -sin60);

  EXPECT_DOUBLE_EQ(normals[50](0), 0.5);
  EXPECT_DOUBLE_EQ(normals[50](1), -sin60);

  EXPECT_DOUBLE_EQ(normals[51](0), 1.0);
  EXPECT_DOUBLE_EQ(normals[51](1), 0.0);

  EXPECT_DOUBLE_EQ(normals[52](0), 1.0);
  EXPECT_DOUBLE_EQ(normals[52](1), 0.0);

  EXPECT_DOUBLE_EQ(normals[53](0), 1.0);
  EXPECT_DOUBLE_EQ(normals[53](1), 0.0);

  EXPECT_DOUBLE_EQ(normals[54](0), 0.5);
  EXPECT_DOUBLE_EQ(normals[54](1), sin60);

  EXPECT_DOUBLE_EQ(normals[55](0), 0.5);
  EXPECT_DOUBLE_EQ(normals[55](1), sin60);

  EXPECT_DOUBLE_EQ(normals[56](0), 0.5);
  EXPECT_DOUBLE_EQ(normals[56](1), sin60);

  EXPECT_DOUBLE_EQ(normals[57](0), -0.5);
  EXPECT_DOUBLE_EQ(normals[57](1), sin60);

  EXPECT_DOUBLE_EQ(normals[58](0), -0.5);
  EXPECT_DOUBLE_EQ(normals[58](1), sin60);

  EXPECT_DOUBLE_EQ(normals[59](0), -0.5);
  EXPECT_DOUBLE_EQ(normals[59](1), sin60);
}

TEST_F(HexagonalLatticeTest, pin_corners)
{
  Real bp = 4.0, pp = 0.8, pd = 0.6, wd = 0.05, wp = 50.0;
  unsigned int nr = 2, a = 2;
  HexagonalLatticeUtils hl(bp, pp, pd, wd, wp, nr, a);
  const auto & pin_corners = hl.pinCenteredCornerCoordinates();
  Real sin60 = std::sqrt(3.0) / 2.0;
  Real cos60 = 0.5;

  // center pin
  double side = 0.46188021535170065;
  const auto & pin0 = pin_corners[0];
  EXPECT_DOUBLE_EQ(pin0[0](0), 0.0);
  EXPECT_DOUBLE_EQ(pin0[0](1), side);

  EXPECT_DOUBLE_EQ(pin0[1](0), -side * sin60);
  EXPECT_DOUBLE_EQ(pin0[1](1), side * cos60);

  EXPECT_DOUBLE_EQ(pin0[2](0), -side * sin60);
  EXPECT_DOUBLE_EQ(pin0[2](1), -side * cos60);

  EXPECT_DOUBLE_EQ(pin0[3](0), (unsigned int)0);
  EXPECT_DOUBLE_EQ(pin0[3](1), -side);

  EXPECT_DOUBLE_EQ(pin0[4](0), side * sin60);
  EXPECT_DOUBLE_EQ(pin0[4](1), -side * cos60);

  EXPECT_DOUBLE_EQ(pin0[5](0), side * sin60);
  EXPECT_DOUBLE_EQ(pin0[5](1), side * cos60);

  // second pin
  const auto & pin_centers = hl.pinCenters();
  const auto & pin1 = pin_corners[1];
  double x = pin_centers[1](0);
  double y = pin_centers[1](1);

  EXPECT_DOUBLE_EQ(pin1[0](0), x + 0.0);
  EXPECT_DOUBLE_EQ(pin1[0](1), y + side);

  EXPECT_DOUBLE_EQ(pin1[1](0), x + -side * sin60);
  EXPECT_DOUBLE_EQ(pin1[1](1), y + side * cos60);

  EXPECT_DOUBLE_EQ(pin1[2](0), x + -side * sin60);
  EXPECT_DOUBLE_EQ(pin1[2](1), y + -side * cos60);

  EXPECT_DOUBLE_EQ(pin1[3](0), x + 0);
  EXPECT_DOUBLE_EQ(pin1[3](1), y + -side);

  EXPECT_DOUBLE_EQ(pin1[4](0), x + side * sin60);
  EXPECT_DOUBLE_EQ(pin1[4](1), y + -side * cos60);

  EXPECT_DOUBLE_EQ(pin1[5](0), x + side * sin60);
  EXPECT_DOUBLE_EQ(pin1[5](1), y + side * cos60);

  // check for bin indexing
  Point p(0.07, 0.195, 0.0);
  EXPECT_EQ(hl.pinIndex(p), (unsigned int)0);

  p = {0.02, 0.42, 0.0};
  EXPECT_EQ(hl.pinIndex(p), (unsigned int)0);

  p = {0.74, 0.46, 0.0};
  EXPECT_EQ(hl.pinIndex(p), (unsigned int)1);

  p = {-0.206, 0.668, 0.0};
  EXPECT_EQ(hl.pinIndex(p), (unsigned int)2);

  p = {-0.99, 0.313, 0.0};
  EXPECT_EQ(hl.pinIndex(p), (unsigned int)3);

  p = {-0.257, -0.67, 0.0};
  EXPECT_EQ(hl.pinIndex(p), (unsigned int)4);

  p = {0.67, -0.79, 0.0};
  EXPECT_EQ(hl.pinIndex(p), (unsigned int)5);

  p = {0.43, 0.0206, 0.0};
  EXPECT_EQ(hl.pinIndex(p), (unsigned int)6);

  p = {0.85, 0.51, 0.0};
  EXPECT_EQ(hl.pinIndex(p), (unsigned int)7);
}

TEST_F(HexagonalLatticeTest, constructor)
{
  Real bp = 4.0, pp = 0.8, pd = 0.6, wd = 0.05, wp = 50.0;
  unsigned int nr = 3, a = 2;
  HexagonalLatticeUtils hl(bp, pp, pd, wd, wp, nr, a);
  EXPECT_DOUBLE_EQ(hl.bundlePitch(), 4.0);
  EXPECT_DOUBLE_EQ(hl.pinPitch(), 0.8);
  EXPECT_DOUBLE_EQ(hl.pinDiameter(), 0.6);
  EXPECT_DOUBLE_EQ(hl.pinRadius(), 0.3);
  EXPECT_DOUBLE_EQ(hl.wireDiameter(), 0.05);
  EXPECT_DOUBLE_EQ(hl.wirePitch(), 50.0);
}
