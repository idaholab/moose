The boost contribs are from boost 1.55, and were selected with the command:
bin.v2/tools/bcp/gcc-4.8.2/release/link-static/bcp boost/math/distributions.hpp boost/math/distributions/*.hpp boost/random/mersenne_twister.hpp boost/numeric/ublas/matrix.hpp boost/numeric/ublas/lu.hpp boost/numeric/odeint.hpp /tmp/foo

and then /tmp/foo/boost was copied into the contrib directory.
