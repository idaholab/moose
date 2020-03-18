      program main

      include 'mpif.h'

      integer ierr, rank, size, len
      character name*(MPI_MAX_PROCESSOR_NAME)

      call MPI_INIT(ierr)
      call MPI_COMM_RANK(MPI_COMM_WORLD, rank, ierr)
      call MPI_COMM_SIZE(MPI_COMM_WORLD, size, ierr)
      call MPI_GET_PROCESSOR_NAME(name, len, ierr)

      print '(2A,I2,A,I2,3A)',
     &      'Hello, World! ',
     &     'I am process ', rank,
     &     ' of ', size,
     &     ' on ', trim(name), '.'

      call MPI_FINALIZE(ierr)

      end
