program main

  use mpi
  implicit none

  integer :: provided,  ierr, size, rank, len
  character (len=MPI_MAX_PROCESSOR_NAME) :: name

  call MPI_Init(ierr)

  call MPI_Comm_rank(MPI_COMM_WORLD, rank, ierr)
  call MPI_Comm_size(MPI_COMM_WORLD, size, ierr)
  call MPI_Get_processor_name(name, len, ierr)

  write(*, '(2A,I2,A,I2,3A)') &
       'Hello, World! ', &
       'I am process ', rank, &
       ' of ', size, &
       ' on ', name(1:len), '.'

  call MPI_Finalize(ierr)

end program main
