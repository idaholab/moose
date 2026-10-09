#!/usr/bin/env python3
"""Generate a 2x-subdivided icosphere as ASCII STL.

Radius 1, origin-centered.  Outward-oriented per right-hand rule so
libMesh's `NON_ORIENTED` check accepts it.

Starting from the 12-vertex icosahedron (20 triangles), subdivide once by
splitting each triangle into 4 (midpoint projection to the unit sphere).
Result: 80 triangles, 42 vertices.
"""

import math
import sys


def normalize(v):
    n = math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])
    return (v[0] / n, v[1] / n, v[2] / n)


def icosahedron():
    """Return (vertices, faces) for a unit-radius icosahedron."""
    t = (1.0 + math.sqrt(5.0)) / 2.0
    raw = [
        (-1, t, 0),
        (1, t, 0),
        (-1, -t, 0),
        (1, -t, 0),
        (0, -1, t),
        (0, 1, t),
        (0, -1, -t),
        (0, 1, -t),
        (t, 0, -1),
        (t, 0, 1),
        (-t, 0, -1),
        (-t, 0, 1),
    ]
    verts = [normalize(v) for v in raw]
    faces = [
        (0, 11, 5),
        (0, 5, 1),
        (0, 1, 7),
        (0, 7, 10),
        (0, 10, 11),
        (1, 5, 9),
        (5, 11, 4),
        (11, 10, 2),
        (10, 7, 6),
        (7, 1, 8),
        (3, 9, 4),
        (3, 4, 2),
        (3, 2, 6),
        (3, 6, 8),
        (3, 8, 9),
        (4, 9, 5),
        (2, 4, 11),
        (6, 2, 10),
        (8, 6, 7),
        (9, 8, 1),
    ]
    return verts, faces


def subdivide(verts, faces):
    """One-level Loop-style subdivision, projecting midpoints to the sphere."""
    cache = {}
    new_verts = list(verts)

    def midpoint(i, j):
        key = (min(i, j), max(i, j))
        if key in cache:
            return cache[key]
        a, b = new_verts[i], new_verts[j]
        m = normalize(((a[0] + b[0]) / 2, (a[1] + b[1]) / 2, (a[2] + b[2]) / 2))
        cache[key] = len(new_verts)
        new_verts.append(m)
        return cache[key]

    new_faces = []
    for a, b, c in faces:
        ab = midpoint(a, b)
        bc = midpoint(b, c)
        ca = midpoint(c, a)
        new_faces.extend([(a, ab, ca), (b, bc, ab), (c, ca, bc), (ab, bc, ca)])
    return new_verts, new_faces


def face_normal(v0, v1, v2):
    ux, uy, uz = v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]
    vx, vy, vz = v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2]
    nx = uy * vz - uz * vy
    ny = uz * vx - ux * vz
    nz = ux * vy - uy * vx
    return normalize((nx, ny, nz))


def write_ascii_stl(path, verts, faces, name="unit_sphere"):
    with open(path, "w") as f:
        f.write(f"solid {name}\n")
        for a, b, c in faces:
            v0, v1, v2 = verts[a], verts[b], verts[c]
            n = face_normal(v0, v1, v2)
            f.write(f"  facet normal {n[0]:.16e} {n[1]:.16e} {n[2]:.16e}\n")
            f.write("    outer loop\n")
            for v in (v0, v1, v2):
                f.write(f"      vertex {v[0]:.16e} {v[1]:.16e} {v[2]:.16e}\n")
            f.write("    endloop\n")
            f.write("  endfacet\n")
        f.write(f"endsolid {name}\n")


def main(out):
    verts, faces = icosahedron()
    verts, faces = subdivide(verts, faces)
    print(f"vertices: {len(verts)}, triangles: {len(faces)}", file=sys.stderr)
    write_ascii_stl(out, verts, faces)


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "unit_sphere.stl")
