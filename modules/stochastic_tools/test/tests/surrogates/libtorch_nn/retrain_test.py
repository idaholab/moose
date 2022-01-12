import unittest
import mooseutils

class TestSkewnessCorrectedStencil(unittest.TestCase):
    def test(self):
        executable = mooseutils.find_moose_executable_recursive(os.getcwd())
        mpi=1
        args = ['Trainrs/train/read_from_file=false']
        print('Running:', executable, ' '.join(args))
        out = mooseutils.run_executable(executable, *args, mpi=mpi, suppress_output=not console)
        args = ['Trainrs/train/read_from_file=true']
        out = mooseutils.run_executable(executable, *args, mpi=mpi, suppress_output=not console)

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
