#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

#
# To run:
#
# 1.) Set the following parameters (see end of script)
# Q = polynomial degree, e.g. 2
# refmin = first reference mesh to generate, e.g. 0
# refmax = last reference mesh to generate, e.g. 1
# TriFlag = True/False depending on whether you want to use triangles.
#
# 2.) In the terminal:
# ./SmoothBump.py
import numpy as np


def writeGMSH_Quad(f, nelem, nBCelem, Q, E):
    '''
    Write quad elements with given parameters.  See writeGMSH()
    function documentation for meaning of parameters.
    '''
    if Q == 1: # 4-node quad
        GmshElemType = 3
        nodemap = (0, 1, 3, 2)
    if Q == 2:  # 9-node second order quad
        GmshElemType = 10
        nodemap = (0, 4, 1, 7, 8, 5, 3, 6, 2)

    # Invert the map
    nodemapinv = []
    for k in xrange((Q+1)*(Q+1)):
        j = 0
        while nodemap[j] != k:
            j += 1
        nodemapinv.append(j)

    for e in xrange(nelem):
        f.write(str(nBCelem+e+1) + ' ' + str(GmshElemType) + ' 2 0 4 ')

        # Write nodes
        for k in xrange((Q+1)*(Q+1)):
            f.write(str(E[e,nodemapinv[k]]))
            # Write a space for every node except last
            if (k != (Q+1)*(Q+1) - 1):
                f.write(' ')
        f.write('\n')



def writeGMSH_Tri(f, nelem, nBCelem, Q, E, NC):
    '''
    Write triangular elements with given parameters.  See writeGMSH()
    function documentation for meaning of parameters.
    '''
    ni = int((NC.shape[0]-1)/Q)
    nj = int((NC.shape[1]-1)/Q)

    if Q == 1: # 3-node triangle
        GmshElemType = 2
        nodemap  = (0,  1,  2, -1)
        nodemap2 = (0,  1, -1,  2)
    if Q == 2:  # 6-node second order triangle
        GmshElemType = 9
        nodemap  = (0,  3,  1,  5,  4, -1,  2, -1, -1)
        nodemap2 = (0,  3,  1, -1,  5,  4, -1, -1,  2)

    # Invert the map
    nodemapinv  = []
    for k in xrange(int((Q+1)*(Q+2)/2)):
        j = 0
        while nodemap[j] != k:
            j += 1
        nodemapinv.append(j)

    nodemapinv2  = []
    for k in xrange(int((Q+1)*(Q+2)/2)):
        j = 0
        while nodemap2[j] != k:
            j += 1
        nodemapinv2.append(j)

    for j in xrange(nj):
        for i in xrange(int(ni/2)):
            e = i + ni*j
            f.write(str(nBCelem+2*e+1) + ' ' + str(GmshElemType) + ' 2 0 4 ')

            # Write nodes
            for k in xrange(int((Q+1)*(Q+2)/2)):
                f.write(str(E[e,nodemapinv[k]])+' ')
            f.write('\n')

            f.write(str(nBCelem+2*e+2) + ' ' + str(GmshElemType) + ' 2 0 4 ')

            # Write nodes
            for k in xrange(int((Q+1)*(Q+2)/2)):
                f.write(str(E[e,(Q+1)*(Q+1)-1-nodemapinv[k]])+' ')
            f.write('\n')

        for i in xrange(int(ni/2),ni):
            e = i + ni*j
            f.write(str(nBCelem+2*e+1) + ' ' + str(GmshElemType) + ' 2 0 4 ')

            # Write nodes
            for k in xrange(int((Q+1)*(Q+2)/2)):
                f.write(str(E[e,nodemapinv2[k]])+' ')
            f.write('\n')

            f.write(str(nBCelem+2*e+2) + ' ' + str(GmshElemType) + ' 2 0 4 ')

            # Write nodes
            for k in xrange(int((Q+1)*(Q+2)/2)):
                f.write(str(E[e,(Q+1)*(Q+1)-1-nodemapinv2[k]])+' ')
            f.write('\n')



def writeGMSH(filename_base, ref, Q, TriFlag, E, V, NC, ni, nj):
    '''
    Write a Gmsh file with the given parameters
    filename_base - name of file, will have other details appended.
    ref - reference number, the higher this is, the more refined the grid is.
    Q - Can be 1 or 2; the order of the elements generated
    TriFlag - if true, generate triangles, else generate quads
    E - 2D connectivity array, passed to writeGMSH_Tri and writeGMSH_Quad
    V - Nodal positions
    NC - node number matrices for writing out blocks
    ni - Number of elements on the wall
    nj - Number of elements on the inflow boundary
    '''
    filename = filename_base + ('_tri' if TriFlag else '_quad') + '_ref' + str(ref) + '_Q' + str(Q) + '.msh'
    print 'Writing ', filename
    f = open(filename, 'w')

    fac = 2 if TriFlag else 1

    nelem = E.shape[0];
    nnode = V.shape[0];

    nInflow = int((nj-1)/Q)
    nOutflow = nInflow
    nWall = int((ni-1)/Q)

    floatformat = "{:3.16e}"

    #Write out the Gmsh file format
    f.write('$MeshFormat\n')
    f.write('2.2 0 8\n')
    f.write('$EndMeshFormat\n')
    f.write('$Nodes\n')
    f.write(str(nnode)+'\n')
    for i in xrange(nnode):
        f.write("{:2d}".format(i+1) + ' ' + floatformat.format(V[i,0]) + ' ' + floatformat.format(V[i,1]) + ' 0.0\n')
    f.write('$EndNodes\n')

    f.write('$Elements\n')
    f.write(str(4 + fac*nelem+nInflow+nOutflow+2*nWall)+'\n')

    #----------------#
    # Corners        #
    #----------------#

    f.write('1 15 2 0 6 ' + str(NC[ 0, 0]) + '\n')
    f.write('2 15 2 0 6 ' + str(NC[-1, 0]) + '\n')
    f.write('3 15 2 0 6 ' + str(NC[-1,-1]) + '\n')
    f.write('4 15 2 0 6 ' + str(NC[ 0,-1]) + '\n')

    if Q == 1: GmshLineType = 1 # 2-node line
    if Q == 2: GmshLineType = 8 # 3-node line

    #----------------#
    # Boundary faces #
    #----------------#

    # Inflow
    BC = 1
    for j in xrange(nInflow):
        f.write(str(j+5) + ' ' + str(GmshLineType) + ' 2 ' + str(BC) + ' 0 ')
        # Write end points
        f.write(str(NC[0,Q*j]) + ' ' + str(NC[0,Q*(j+1)]) + ' ')
        # Write higher-order nodes
        for q in xrange(1,Q):
            f.write(str(NC[0,Q*j+q]))
            # Avoid writing trailing whitespace
            if (q < Q-1):
                f.write(' ')
        f.write('\n')

    # Outflow
    BC = 2
    for j in xrange(nOutflow):
        f.write(str(nInflow+j+5) + ' ' + str(GmshLineType) + ' 2 ' + str(BC) + ' 0 ')
        # Write end points
        f.write(str(NC[ni-1,Q*j]) + ' ' + str(NC[ni-1,Q*(j+1)]) + ' ')
        # Write higher-order nodes
        for q in xrange(1,Q):
            f.write(str(NC[ni-1,Q*j+q]))
            # Avoid writing trailing whitespace
            if (q < Q-1):
                f.write(' ')
        f.write('\n')

    # Walls
    BC = 3
    for i in xrange(nWall):
        f.write(str(nInflow+nOutflow+i+5) + ' ' + str(GmshLineType) + ' 2 ' + str(BC) + ' 0 ')
        # Write end points
        f.write(str(NC[Q*i,0]) + ' ' + str(NC[Q*(i+1),0]) + ' ')
        # Write higher-order nodes
        for q in xrange(1,Q):
            f.write(str(NC[Q*i+q,0]))
            # Avoid writing trailing whitespace
            if (q < Q-1):
                f.write(' ')
        f.write('\n')

    BC = 4
    for i in xrange(nWall):
        f.write(str(nInflow+nOutflow+nWall+i+5) + ' ' + str(GmshLineType) + ' 2 ' + str(BC) + ' 0 ')
        # Write end points
        f.write(str(NC[Q*i,nj-1]) + ' ' + str(NC[Q*(i+1),nj-1]) + ' ')
        # Write higher-order nodes
        for q in xrange(1,Q):
            f.write(str(NC[Q*i+q,nj-1]))
            # Avoid writing trailing whitespace
            if (q < Q-1):
                f.write(' ')
        f.write('\n')

    nBCelem = 2*nWall + nInflow + nOutflow

    if TriFlag:
        writeGMSH_Tri(f, nelem, nBCelem, Q, E, NC)
    else:
        writeGMSH_Quad(f, nelem, nBCelem, Q, E)

    f.write('$EndElements\n')
    f.write('$PhysicalNames\n')
    f.write('5\n')
    f.write('2 0 \"MeshInterior\"\n')
    f.write('1 1 \"Inflow\"\n')
    f.write('1 2 \"Outflow\"\n')
    f.write('1 3 \"Lower Wall\"\n')
    f.write('1 4 \"Upper Wall\"\n')
    f.write('$EndPhysicalNames\n')
    f.close()



def block_elem(N, Q):
    '''
    Function which computes the Element connectivity block E, given N and Q.
    '''
    nx, ny = N.shape;
    mx = int((nx-1)/Q);
    my = int((ny-1)/Q);
    E = np.zeros((mx*my,(Q+1)*(Q+1)), int);
    dy = ny;
    i = 0;
    for imy in xrange(my):
        for imx in xrange(mx):
            ix = Q*(imx+1)-(Q-1)-1;
            iy = Q*(imy+1)-(Q-1)-1;
            k = 0;
            for ky in xrange(Q+1):
                for kx in xrange(Q+1):
                    E[i,k] = N[ix+kx,iy+ky]
                    k = k+1;

            i = i + 1;

    return E



def SmoothBump(ni, nj, Q, ref, TriFlag):
    '''
    Driver function that sets a few parameters and calls writeGMSH.
    '''
    fac = 2 if TriFlag else 1

    ni = ni*Q*2**ref+1
    nj = nj*Q*2**ref+1

    print 'Cell size ' + str(int((ni-1)/Q)) + 'x' + str(int((nj-1)/2)) + ' with ' + str(fac*int((ni-1)/Q)*int((nj-1)/2)) + ' Elements'

    # Create all the vertexes
    V = np.zeros((ni,nj,2), float)

    # Upper boundary
    y1 = 0.8
    x0 = np.linspace(-1.5, 1.5, ni)

    for i in xrange(ni):
        x = x0[i];
        y0 = 0.0625*np.exp(-25.*x**2)
        y = np.linspace(y0, y1, nj)
        V[i,:,0] = x
        V[i,:,1] = y

    V = V.reshape( (ni*nj,2) )

    # node number matrices for writing out blocks
    NC = np.arange(ni*nj).reshape((ni, nj)) + 1

    # form elements
    E = block_elem(NC, Q);

    writeGMSH('SmoothBump', ref, Q, TriFlag, E, V, NC, ni, nj)





# Q is the degree of the polynomial used to represent elements.
# Q=1: linear/bilinear
# Q=2: quadratic/biquadratic
Q = 2

# The range of reference meshes to generate.  If refmin==refmax, only
# one mesh with that reference number will be generated.
refmin = 2
refmax = 2

# Set to True for triangle grids, and False for quads
TriFlag = False

for ref in xrange(refmin, refmax+1):
    SmoothBump(ni=6, nj=2, Q=Q, ref=ref, TriFlag=TriFlag)


# Local Variables:
# python-indent: 2
# End:
