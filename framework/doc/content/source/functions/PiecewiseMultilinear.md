# PiecewiseMultilinear

The `PiecewiseMultilinear` function provides the capability of multi-dimensional
piecewise linear interpolation. The data is read from file provided in the
`data_file` argument. Interpolation axes can be a selection of one or multiple of
`x`, `y`, `z`, `t`, the number of interpolation axes is referred to as dimension
of the interpolation.

Formatting instructions for the `data_file` (reproduced from `data_file` docstring):
any empty line and any line
beginning with # are ignored, all other lines are assumed to contain relevant information.
The file must begin with specification of the grid.  This is done through lines containing
the keywords: AXIS X; AXIS Y; AXIS Z; or AXIS T.  Immediately following the keyword line
must be a space-separated line of real numbers which define the grid along the specified
axis.  These data must be monotonically increasing.  After all the axes and their grids
have been specified, there must be a line that is DATA.  Following that line, function
values are given in the correct order (they may be on individual lines, or be
space-separated on a number of lines).  When the function is evaluated, f(i,j,k,l)
corresponds to the i + j*Ni + k*Ni*Nj + l*Ni*Nj*Nk data value.  Here i>=0 corresponding to
the index along the first AXIS, j>=0 corresponding to the index along the second AXIS, etc,
and Ni = number of grid points along the first AXIS, etc.

!syntax description /Functions/PiecewiseMultilinear

!syntax parameters /Functions/PiecewiseMultilinear

!syntax inputs /Functions/PiecewiseMultilinear

!syntax children /Functions/PiecewiseMultilinear

!bibtex bibliography
