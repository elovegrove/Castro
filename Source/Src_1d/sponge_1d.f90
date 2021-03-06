module sponge_module

  implicit none

contains

  subroutine sponge(uout, uout_l1, uout_h1, lo, hi, time, dt, &
       dx, domlo, domhi)
    
    use meth_params_module, only : NVAR
    
    implicit none
    integer          :: lo(1), hi(1), domlo(1), domhi(1)
    integer          ::  uout_l1, uout_h1
    double precision :: uout(uout_l1:uout_h1,NVAR)
    double precision :: time,dt
    double precision :: dx
    
    ! Nothing happens in this generic version of the sponge routine.
    
  end subroutine sponge

end module sponge_module
