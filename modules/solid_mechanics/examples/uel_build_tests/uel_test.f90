      subroutine uel(rhs,amatrx,svars,energy,ndofel,nrhs,nsvars,    &
       props,nprops,coords,mcrd,nnode,u,du,v,a,jtype,time,dtime,    &
       kstep,kinc,jelem,params,ndload,jdltyp,adlmag,predef,npredf,  &
       lflags,mlvarx,ddlmag,mdload,pnewdt,jprops,njprop,period)
!
! Purpose:
!    Call the user element subroutine
!
!
! Revisions:
!     Date                   Programmer               Description of change
!
!      include 'aba_param.inc'
       implicit double precision(a-h,o-z)
!       implicit none
! Data dictionary: declare calling parameters types and definitions
      integer,      intent(in)                            :: mlvarx ! dimensioning parameter for rhs
      integer,      intent(in)                            :: ndofel ! number of dof of the element
      integer,      intent(in)                            :: nrhs   ! number of load vectors
      integer,      intent(in)                            :: nsvars ! number of solution state variables
      integer,      intent(in)                            :: nprops ! number of the property values associated with the element
      integer,      intent(in)                            :: mcrd   ! maximum of the user-defined maximum number of coordinates needed at any node point
      integer,      intent(in)                            :: nnode  ! number of nodes on the element
      integer,      intent(in)                            :: jtype  ! defines the element type
      integer,      intent(in)                            :: kstep  ! current step number
      integer,      intent(in)                            :: kinc   ! current increment number
      integer,      intent(in)                            :: jelem  ! User-assigned element number
      integer,      intent(in)                            :: ndload ! Identification number of the distributed load or flux currently active on this element
      integer,      intent(in)                            :: mdload ! Total number of distributed loads and/or fluxes defined on this element
      integer,      intent(in)                            :: npredf ! Number of predefined field variables
      integer,      intent(in)                            :: njprop ! User-defined number of integer property values associated with the element
      integer,      intent(in),  dimension(*)             :: lflags ! contains the flags that define the current solution procedure and requirements for elements calculations
      integer,      intent(in),  dimension(mdload,*)      :: jdltyp ! contains the integers used to define distributed load types for the element
      integer,      intent(in),  dimension(*)             :: jprops ! contains the NJPROP integer property values defined for use with this element
	real(kind=8), intent(in)                            :: dtime  ! time increment
	real(kind=8), intent(in)                            :: period ! Time period of the current step
      real(kind=8), intent(in),  dimension(2,npredf,nnode):: predef ! contains the values of predefined field variables
      real(kind=8), intent(in),  dimension(nprops)             :: props  ! contains the NPROPS real property values defined for use with this element
      real(kind=8), intent(in),  dimension(mcrd,nnode)    :: coords	! contains the original coordinates of the nodes of the element
      real(kind=8), intent(in),  dimension(ndofel)        :: u      ! contains the current estimate of the total values of the variables
      real(kind=8), intent(in),  dimension(mlvarx,*)      :: du     ! contains the incremental values of the variables for the current increment for right-hand-side KRHS
      real(kind=8), intent(in),  dimension(ndofel)        :: v      ! contains the time rate of change of the variables
      real(kind=8), intent(in),  dimension(ndofel)        :: a      ! contains the accelerations of the variables
      real(kind=8), intent(in),  dimension(2)             :: time   ! time(1) Current value of step time or frequency, time(2) Current value of total time.
      real(kind=8), intent(in),  dimension(*)             :: params ! contains the parameters associated with the solution procedure
      real(kind=8), intent(in),  dimension(mdload,*)      :: ddlmag ! contains the increments in the magnitudes of the distributed loads that are currently active on this element

	real(kind=8), intent(out), dimension(mlvarx,*)      :: rhs    ! contains the contributions of this element to the rhs vector
	real(kind=8), intent(out), dimension(ndofel,ndofel) :: amatrx ! contains the contributions of this element to the jacobian
	real(kind=8), intent(out), dimension(*)             :: svars  ! contains the values of the solution dependent state variables
	real(kind=8), intent(out), dimension(8)             :: energy ! contains the values of the energy quantities associated with the element
	real(kind=8), intent(inout)                         :: pnewdt ! Ratio of suggested new time increment to the time increment currently being used
	real(kind=8), intent(inout), dimension(mdload,*)    :: adlmag ! Total load magnitude of the K1th distributed load at the end of the current increment..

! Element number from model.
      write(*,*) 'Element number is: ', jelem

! Maximum number of coordinates needed at a node
      write(*,*) 'Number of coordinates at a node: ', mcrd

! Printing some information to test initial interface MOOSE-UEL
      write(*,*) 'Element type is: ', jtype

! Test properties passed to UEL (likely those possibly passed to a UMAT will be a subset of these)
      write(*,*) 'Number of UEL properties is: ', nprops
      do i=1,nprops
        write(*,*) 'Property ', i, ' is: ', props(i)
      enddo

! Number of nodes in this element
      write(*,*) 'Number of nodes in this element: ', nnode

! State variables passed to UEL (again, likely those for a UMAT routine will be a subset of these)
      write(*,*) 'Number of state variables is: ', nsvars
      do i=1,nsvars
            write(*,*) 'State variable ', i, ' is: ', svars(i)
      enddo

! Predefined variables. Typically temperature first, then others
      write(*,*) 'Number of predefined variables: ', npredf
      do i=1,npredf
        do j=1,nnode
            write(*,*) 'External field number ', i, ' for node ', j, ' is: ', predef(1,i,j)
        enddo
      enddo

! Time increment
      write(*,*) 'Time increment is: ', dtime

! Print coordinates of the element
      do i=1,nnode
        write(*,*) 'Coordinates for node ', i, ' are: ', coords(1,i), ', ', coords(2,i), ', ', coords(3,i)
      enddo


      return
      end subroutine uel
