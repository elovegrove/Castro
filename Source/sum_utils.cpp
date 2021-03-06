#include <iomanip>

#include <Castro.H>
#include <Castro_F.H>

#ifdef GRAVITY
#include <Gravity.H>
#include <Gravity_F.H>
#endif

Real
Castro::sumDerive (const std::string& name,
                   Real           time)
{
    Real sum     = 0.0;
    MultiFab* mf = derive(name, time, 0);

    BL_ASSERT(!(mf == 0));

    if (level < parent->finestLevel())
    {
	const MultiFab* mask = getLevel(level+1).build_fine_mask();
	MultiFab::Multiply(*mf, *mask, 0, 0, 1, 0);
    }

#ifdef _OPENMP
#pragma omp parallel reduction(+:sum)
#endif
    {
	for (MFIter mfi(*mf,true); mfi.isValid(); ++mfi)
	{
	    sum += (*mf)[mfi].sum(mfi.tilebox(),0);
	}
    }

    delete mf;

    ParallelDescriptor::ReduceRealSum(sum);

    return sum;
}

Real
Castro::volWgtSum (const std::string& name,
                   Real               time)
{
    BL_PROFILE("Castro::volWgtSum()");

    Real        sum     = 0.0;
    const Real* dx      = geom.CellSize();
    MultiFab*   mf      = derive(name,time,0);

    BL_ASSERT(mf != 0);

    if (level < parent->finestLevel())
    {
	const MultiFab* mask = getLevel(level+1).build_fine_mask();
	MultiFab::Multiply(*mf, *mask, 0, 0, 1, 0);
    }

#ifdef _OPENMP
#pragma omp parallel reduction(+:sum)
#endif    
    for (MFIter mfi(*mf,true); mfi.isValid(); ++mfi)
    {
        FArrayBox& fab = (*mf)[mfi];

	Real s = 0.0;
        const Box& box  = mfi.tilebox();
        const int* lo   = box.loVect();
        const int* hi   = box.hiVect();
#if(BL_SPACEDIM < 3) 
	const int i     = mfi.index();
        const Real* rad = radius[i].dataPtr();
	const int* vlo  =  grids[i].loVect();
	const int* vhi  =  grids[i].hiVect();
        int irlo        = vlo[0]-radius_grow;
        int irhi        = vhi[0]+radius_grow;
#endif

        //
        // Note that this routine will do a volume weighted sum of
        // whatever quantity is passed in, not strictly the "mass".
        //
#if(BL_SPACEDIM == 1) 
	BL_FORT_PROC_CALL(CA_SUMMASS,ca_summass)
            (BL_TO_FORTRAN(fab),lo,hi,dx,&s,rad,irlo,irhi);
#elif(BL_SPACEDIM == 2)
	BL_FORT_PROC_CALL(CA_SUMMASS,ca_summass)
            (BL_TO_FORTRAN(fab),lo,hi,dx,&s,rad,irlo,irhi);
#elif(BL_SPACEDIM == 3)
	BL_FORT_PROC_CALL(CA_SUMMASS,ca_summass)
            (BL_TO_FORTRAN(fab),lo,hi,dx,&s);
#endif
        sum += s;
    }

    delete mf;

    ParallelDescriptor::ReduceRealSum(sum);

    return sum;
}

Real
Castro::volWgtSquaredSum (const std::string& name,
                          Real               time)
{
    BL_PROFILE("Castro::volWgtSquaredSum()");

    Real        sum     = 0.0;
    const Real* dx      = geom.CellSize();
    MultiFab*   mf      = derive(name,time,0);

    BL_ASSERT(mf != 0);

    if (level < parent->finestLevel())
    {
	const MultiFab* mask = getLevel(level+1).build_fine_mask();
	MultiFab::Multiply(*mf, *mask, 0, 0, 1, 0);
    }

#ifdef _OPENMP
#pragma omp parallel reduction(+:sum)
#endif    
    for (MFIter mfi(*mf,true); mfi.isValid(); ++mfi)
    {
        FArrayBox& fab = (*mf)[mfi];
    
        Real s = 0.0;
        const Box& box  = mfi.tilebox();
        const int* lo   = box.loVect();
        const int* hi   = box.hiVect();
#if(BL_SPACEDIM < 3) 
	const int i     = mfi.index();
        const Real* rad = radius[i].dataPtr();
	const int* vlo  =  grids[i].loVect();
	const int* vhi  =  grids[i].hiVect();
        int irlo        = vlo[0]-radius_grow;
        int irhi        = vhi[0]+radius_grow;
#endif

        //
        // Note that this routine will do a volume weighted sum of
        // whatever quantity is passed in, not strictly the "mass".
        //
#if(BL_SPACEDIM == 1) 
	BL_FORT_PROC_CALL(CA_SUMSQUARED,ca_sumsquared)
            (BL_TO_FORTRAN(fab),lo,hi,dx,&s,rad,irlo,irhi);
#elif(BL_SPACEDIM == 2)
	BL_FORT_PROC_CALL(CA_SUMSQUARED,ca_sumsquared)
            (BL_TO_FORTRAN(fab),lo,hi,dx,&s,rad,irlo,irhi);
#elif(BL_SPACEDIM == 3)
	BL_FORT_PROC_CALL(CA_SUMSQUARED,ca_sumsquared)
            (BL_TO_FORTRAN(fab),lo,hi,dx,&s);
#endif
        sum += s;
    }

    delete mf;

    ParallelDescriptor::ReduceRealSum(sum);

    return sum;
}

Real
Castro::locWgtSum (const std::string& name,
                   Real               time,
                   int                idir)
{
    BL_PROFILE("Castro::locWgtSum()");

    Real        sum     = 0.0;
    const Real* dx      = geom.CellSize();
    MultiFab*   mf      = derive(name,time,0);

    BL_ASSERT(mf != 0);

    if (level < parent->finestLevel())
    {
	const MultiFab* mask = getLevel(level+1).build_fine_mask();
	MultiFab::Multiply(*mf, *mask, 0, 0, 1, 0);
    }

#ifdef _OPENMP
#pragma omp parallel reduction(+:sum)
#endif    
    for (MFIter mfi(*mf,true); mfi.isValid(); ++mfi)
    {
        FArrayBox& fab = (*mf)[mfi];
    
        Real s = 0.0;
        const Box& box  = mfi.tilebox();
        const int* lo   = box.loVect();
        const int* hi   = box.hiVect();
#if (BL_SPACEDIM < 3)
	const int i     = mfi.index();
        const Real* rad = radius[i].dataPtr();
	const int* vlo  =  grids[i].loVect();
	const int* vhi  =  grids[i].hiVect();
        int irlo        = vlo[0]-radius_grow;
        int irhi        = vhi[0]+radius_grow;
#endif

        //
        // Note that this routine will do a volume weighted sum of
        // whatever quantity is passed in, not strictly the "mass".
        //
#if (BL_SPACEDIM == 1) 
	BL_FORT_PROC_CALL(CA_SUMLOCMASS,ca_sumlocmass)
            (BL_TO_FORTRAN(fab),lo,hi,geom.ProbLo(),dx,&s,rad,irlo,irhi,idir);
#elif (BL_SPACEDIM == 2)
        int geom_flag = Geometry::IsRZ() ? 1 : 0;
        if (idir == 0 && geom_flag == 1) {
            s = 0.0;
        } else {
	   BL_FORT_PROC_CALL(CA_SUMLOCMASS,ca_sumlocmass)
               (BL_TO_FORTRAN(fab),lo,hi,geom.ProbLo(),dx,&s,rad,irlo,irhi,idir);
        }
#else
	BL_FORT_PROC_CALL(CA_SUMLOCMASS,ca_sumlocmass)
            (BL_TO_FORTRAN(fab),lo,hi,geom.ProbLo(),dx,&s,idir);
#endif
        sum += s;
    }

    delete mf;

    ParallelDescriptor::ReduceRealSum(sum);

    return sum;
}

#if (BL_SPACEDIM > 1)
Real
Castro::locWgtSum2D (const std::string& name,
                     Real               time,
                     int                idir1,
                     int                idir2)
{
    BL_PROFILE("Castro::locWgtSum2D()");

    Real        sum     = 0.0;
    const Real* dx      = geom.CellSize();
    MultiFab*   mf      = derive(name,time,0);

    BL_ASSERT(mf != 0);

    if (level < parent->finestLevel())
    {
	const MultiFab* mask = getLevel(level+1).build_fine_mask();
	MultiFab::Multiply(*mf, *mask, 0, 0, 1, 0);
    }

#ifdef _OPENMP
#pragma omp parallel reduction(+:sum)
#endif    
    for (MFIter mfi(*mf,true); mfi.isValid(); ++mfi)
    {
        FArrayBox& fab = (*mf)[mfi];
    
        Real s = 0.0;
        const Box& box  = mfi.tilebox();
        const int* lo   = box.loVect();
        const int* hi   = box.hiVect();
#if (BL_SPACEDIM < 3)
	const int i     = mfi.index();
        const Real* rad = radius[i].dataPtr();
	const int* vlo  =  grids[i].loVect();
	const int* vhi  =  grids[i].hiVect();
        int irlo        = vlo[0]-radius_grow;
        int irhi        = vhi[0]+radius_grow;
#endif

        //
        // Note that this routine will do a volume weighted sum of
        // whatever quantity is passed in, not strictly the "mass".
        //
#if (BL_SPACEDIM == 2)
	BL_FORT_PROC_CALL(CA_SUMLOCMASS2D,ca_sumlocmass2d)
	  (BL_TO_FORTRAN(fab),lo,hi,geom.ProbLo(),dx,&s,rad,irlo,irhi,idir1,idir2);
#elif (BL_SPACEDIM == 3)
	BL_FORT_PROC_CALL(CA_SUMLOCMASS2D,ca_sumlocmass2d)
	  (BL_TO_FORTRAN(fab),lo,hi,geom.ProbLo(),dx,&s,idir1,idir2);
#endif
        sum += s;
    }

    delete mf;

    ParallelDescriptor::ReduceRealSum(sum);

    return sum;
}
#endif

Real
Castro::volWgtSumMF (MultiFab* mf, int comp) 
{
    BL_PROFILE("Castro::volWgtSumMF()");

    Real        sum     = 0.0;
    const Real* dx      = geom.CellSize();

    BL_ASSERT(mf != 0);

#ifdef _OPENMP
#pragma omp parallel reduction(+:sum)
#endif    
    for (MFIter mfi(*mf,true); mfi.isValid(); ++mfi)
    {
        FArrayBox& fab = (*mf)[mfi];

        Real s = 0.0;
        const Box& box  = mfi.tilebox();
        const int* lo   = box.loVect();
        const int* hi   = box.hiVect();
#if (BL_SPACEDIM < 3) 
	const int i     = mfi.index();
        const Real* rad = radius[i].dataPtr();
	const int* vlo  =  grids[i].loVect();
	const int* vhi  =  grids[i].hiVect();
        int irlo        = vlo[0]-radius_grow;
        int irhi        = vhi[0]+radius_grow;
#endif

        //
        // Note that this routine will do a volume weighted sum of
        // whatever quantity is passed in, not strictly the "mass".
        //
#if(BL_SPACEDIM == 1) 
	BL_FORT_PROC_CALL(CA_SUMMASS,ca_summass)
            (BL_TO_FORTRAN_N(fab,comp),lo,hi,dx,&s,rad,irlo,irhi);
#elif(BL_SPACEDIM == 2)
	BL_FORT_PROC_CALL(CA_SUMMASS,ca_summass)
            (BL_TO_FORTRAN_N(fab,comp),lo,hi,dx,&s,rad,irlo,irhi);
#elif(BL_SPACEDIM == 3)
	BL_FORT_PROC_CALL(CA_SUMMASS,ca_summass)
            (BL_TO_FORTRAN_N(fab,comp),lo,hi,dx,&s);
#endif
        sum += s;
    }

    ParallelDescriptor::ReduceRealSum(sum);

    return sum;
}

#if (BL_SPACEDIM > 1)
Real
Castro::volWgtSumOneSide (const std::string& name,
                          Real               time, 
                          int                side,
                          int                bdir)
{
    BL_PROFILE("Castro::volWgtSumOneSide()");

    // This function is a clone of volWgtSum except it computes the result only on half of the domain.
    // The lower half corresponds to side == 0 and the upper half corresponds to side == 1.
    // The argument bdir gives the direction along which to bisect.
    // Only designed to work in Cartesian geometries.

    Real        sum     = 0.0;
    const Real* dx      = geom.CellSize();
    MultiFab*   mf      = derive(name,time,0);
    const int* domhi    = geom.Domain().hiVect();

    BL_ASSERT(mf != 0);

    if (level < parent->finestLevel())
    {
	const MultiFab* mask = getLevel(level+1).build_fine_mask();
	MultiFab::Multiply(*mf, *mask, 0, 0, 1, 0);
    }

#ifdef _OPENMP
#pragma omp parallel reduction(+:sum)
#endif    
    for (MFIter mfi(*mf,true); mfi.isValid(); ++mfi)
    {
        FArrayBox& fab = (*mf)[mfi];
    
        Real s = 0.0;
        const Box& box  = mfi.tilebox();
        const int* lo   = box.loVect();
        const int* hi   = box.hiVect();
#if(BL_SPACEDIM < 3) 
	const int i     = mfi.index();
        const Real* rad = radius[i].dataPtr();
	const int* vlo  =  grids[i].loVect();
	const int* vhi  =  grids[i].hiVect();
        int irlo        = vlo[0]-radius_grow;
        int irhi        = vhi[0]+radius_grow;
#endif

#if (BL_SPACEDIM == 2)
        int hiLeft[2]       = { *hi, *(hi+1) };
        int loRight[2]      = { *lo, *(lo+1) };
#elif (BL_SPACEDIM == 3)
        int hiLeft[3]       = { *hi, *(hi+1), *(hi+2) };
        int loRight[3]      = { *lo, *(lo+1), *(lo+2) };
#endif

        hiLeft[bdir]        = *(domhi+bdir) / 2;
        loRight[bdir]       = *(domhi+bdir) / 2 + 1;

        const int* hiLeftPtr  = hiLeft;
        const int* loRightPtr = loRight;
        const int* loFinal;
        const int* hiFinal;

        //
        // Note that this routine will do a volume weighted sum of
        // whatever quantity is passed in, not strictly the "mass".
        //
        
        bool doSum = false;
        if ( side == 0 && *(lo + bdir) <= *(hiLeftPtr + bdir) ) {
          doSum = true;
          if ( *(hi + bdir) <= *(hiLeftPtr + bdir) ) {
            loFinal   = lo;
            hiFinal   = hi;
	  }
	  else {
            loFinal   = lo;
            hiFinal   = hiLeftPtr;
	  }
	}  
        else if ( side == 1 && *(hi + bdir) >= *(loRightPtr + bdir) ) {
          doSum = true;
          if ( *(lo + bdir) >= *(loRightPtr + bdir) ) {
            loFinal   = lo;
            hiFinal   = hi;
          }
          else {
            loFinal   = loRightPtr;
            hiFinal   = hi;
          }
	}

        if ( doSum ) {
#if (BL_SPACEDIM == 2)
          BL_FORT_PROC_CALL(CA_SUMMASS,ca_summass)
            (BL_TO_FORTRAN(fab),loFinal,hiFinal,dx,&s,rad,irlo,irhi);
#elif (BL_SPACEDIM == 3)
          BL_FORT_PROC_CALL(CA_SUMMASS,ca_summass)
            (BL_TO_FORTRAN(fab),loFinal,hiFinal,dx,&s);
#endif
        }
        
        sum += s;
		
    }

    delete mf;

    ParallelDescriptor::ReduceRealSum(sum);

    return sum;
}

Real
Castro::locWgtSumOneSide (const std::string& name,
                          Real               time,
                          int                idir, 
                          int                side,
                          int                bdir)
{
    BL_PROFILE("Castro::locWgtSumOneSide()");

  // This function is a clone of locWgtSum except that it only sums over one half of the domain.
  // The lower half corresponds to side == 0, and the upper half corresponds to side == 1.
  // The argument idir (x == 0, y == 1, z == 2) gives the direction to location weight by,
  // and the argument bdir gives the direction along which to bisect.

    Real sum            = 0.0;
    const Real* dx      = geom.CellSize();
    MultiFab*   mf      = derive(name,time,0); 
    const int* domhi    = geom.Domain().hiVect(); 

    BL_ASSERT(mf != 0);

    if (level < parent->finestLevel())
    {
	const MultiFab* mask = getLevel(level+1).build_fine_mask();
	MultiFab::Multiply(*mf, *mask, 0, 0, 1, 0);
    }

#ifdef _OPENMP
#pragma omp parallel reduction(+:sum)
#endif        
    for (MFIter mfi(*mf,true); mfi.isValid(); ++mfi)
    {
        FArrayBox& fab = (*mf)[mfi];

        Real s = 0.0;
        const Box& box  = mfi.tilebox();
        const int* lo   = box.loVect();
        const int* hi   = box.hiVect();
#if(BL_SPACEDIM < 3) 
	const int i     = mfi.index();
        const Real* rad = radius[i].dataPtr();
	const int* vlo  =  grids[i].loVect();
	const int* vhi  =  grids[i].hiVect();
        int irlo        = vlo[0]-radius_grow;
        int irhi        = vhi[0]+radius_grow;
#endif

#if (BL_SPACEDIM == 2)
        int hiLeft[2]       = { *hi, *(hi+1) };
        int loRight[2]      = { *lo, *(lo+1) };
#elif (BL_SPACEDIM == 3)
        int hiLeft[3]       = { *hi, *(hi+1), *(hi+2) };
        int loRight[3]      = { *lo, *(lo+1), *(lo+2) };
#endif
        hiLeft[bdir]        = *(domhi+bdir) / 2;
        loRight[bdir]       = (*(domhi+bdir) / 2) + 1;

        const int* hiLeftPtr  = hiLeft;
        const int* loRightPtr = loRight;
        const int* loFinal;
        const int* hiFinal;        
        //
        // Note that this routine will do a volume weighted sum of
        // whatever quantity is passed in, not strictly the "mass".
        // 

        bool doSum = false;
        if ( side == 0 && *(lo + bdir) <= *(hiLeftPtr + bdir) ) {
          doSum = true;
          if ( *(hi + bdir) <= *(hiLeftPtr + bdir) ) {
            loFinal   = box.loVect();
            hiFinal   = box.hiVect();
	  }
	  else {
            loFinal   = box.loVect();
            hiFinal   = hiLeftPtr;
	  }
	}  
        else if ( side == 1 && *(hi + bdir) >= *(loRightPtr + bdir) ) {
          doSum = true;
          if ( *(lo + bdir) >= *(loRightPtr + bdir) ) {
            loFinal   = box.loVect();
            hiFinal   = box.hiVect();
          }
          else {
            loFinal   = loRightPtr;
            hiFinal   = box.hiVect();
          }
	}

        if ( doSum ) {
#if (BL_SPACEDIM == 2)
          BL_FORT_PROC_CALL(CA_SUMLOCMASS,ca_sumlocmass)
             (BL_TO_FORTRAN(fab),loFinal,hiFinal,geom.ProbLo(),dx,&s,rad,irlo,irhi,idir);
#elif (BL_SPACEDIM == 3)
          BL_FORT_PROC_CALL(CA_SUMLOCMASS,ca_sumlocmass)
             (BL_TO_FORTRAN(fab),loFinal,hiFinal,geom.ProbLo(),dx,&s,idir);
#endif
        }
     
        sum += s;
        
    }

    delete mf;

    ParallelDescriptor::ReduceRealSum(sum);

    return sum;

}
#endif

Real
Castro::volProductSum (const std::string& name1, 
                       const std::string& name2,
                       Real time)
{
    BL_PROFILE("Castro::volProductSum()");

    Real        sum     = 0.0;
    const Real* dx      = geom.CellSize();
    MultiFab*   mf1;
    MultiFab*   mf2;

    if (name1 == "phi" || name1 == "PHI" || name1 == "Phi" || name1 == "phiGrav")
    {
      BoxLib::Abort("volProductSum can only have phi be in the second string location.");
    }
    else
      mf1 = derive(name1,time,0);
      
    if (name2 == "phi" || name2 == "PHI" || name2 == "Phi" || name1 == "phiGrav")
    {
#ifdef GRAVITY
      mf2 = gravity->get_phi_curr(level);
#else
      BoxLib::Abort("Phi does not exist when gravity is not turned on.");
#endif
    }
    else
      mf2 = derive(name2,time,0);
    

    BL_ASSERT(mf1 != 0);
    BL_ASSERT(mf2 != 0);

    if (level < parent->finestLevel())
    {
	const MultiFab* mask = getLevel(level+1).build_fine_mask();
	MultiFab::Multiply(*mf1, *mask, 0, 0, 1, 0);
    }

#ifdef _OPENMP
#pragma omp parallel reduction(+:sum)
#endif    
    for (MFIter mfi(*mf1,true); mfi.isValid(); ++mfi)
    {
        FArrayBox& fab1 = (*mf1)[mfi];
        FArrayBox& fab2 = (*mf2)[mfi];
    
        Real s = 0.0;
        const Box& box  = mfi.tilebox();
        const int* lo   = box.loVect();
        const int* hi   = box.hiVect();

	BL_FORT_PROC_CALL(CA_SUMPRODUCT,ca_sumproduct)
	  (BL_TO_FORTRAN(fab1),BL_TO_FORTRAN(fab2),lo,hi,dx,&s);
        
        sum += s;
    }

    if ( name1 != "phi" )
      delete mf1;
    if ( name2 != "phi" )
      delete mf2;

    ParallelDescriptor::ReduceRealSum(sum);

    return sum;
}

#if (BL_SPACEDIM == 3)
Real
Castro::locSquaredSum (const std::string& name,
                       Real               time,
                       int                idir)
{
    BL_PROFILE("Castro::locSquaredSum()");

    Real        sum     = 0.0;
    const Real* dx      = geom.CellSize();
    MultiFab*   mf      = derive(name,time,0);

    BL_ASSERT(mf != 0);

    if (level < parent->finestLevel())
    {
	const MultiFab* mask = getLevel(level+1).build_fine_mask();
	MultiFab::Multiply(*mf, *mask, 0, 0, 1, 0);
    }

#ifdef _OPENMP
#pragma omp parallel reduction(+:sum)
#endif    
    for (MFIter mfi(*mf,true); mfi.isValid(); ++mfi)
    {
        FArrayBox& fab = (*mf)[mfi];
    
        Real s = 0.0;
        const Box& box  = mfi.tilebox();
        const int* lo   = box.loVect();
        const int* hi   = box.hiVect();

	BL_FORT_PROC_CALL(CA_SUMLOCSQUAREDMASS,ca_sumlocsquaredmass)
            (BL_TO_FORTRAN(fab),lo,hi,geom.ProbLo(),dx,&s,idir);

        sum += s;
    }

    delete mf;

    ParallelDescriptor::ReduceRealSum(sum);

    return sum;
}
#endif
