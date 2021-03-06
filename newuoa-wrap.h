	#ifndef LOPTI_NEWUOA_H
	#define LOPTI_NEWUOA_H

	#include <lopti/lopti.h>
	#include <lopti/object_function.h>
	#include <cassert>
	
	#ifndef   MINIMIZER                                                                                                                                  
		#define	MINIMIZER	newuoa_minimizer
	#endif

	#ifndef		V_IBEGIN
		#define	V_IBEGIN 1
	#endif
	

	// reverse indexes for fortran compat
	#define		BMAT(i,j)	BMAT[j][i]
	#define		ZMAT(i,j)	ZMAT[j][i]
	#define		XPT(i,j)	XPT[j][i]

	#include        <cmath>
			using   std::sqrt;  
	#include        <algorithm>
			using   std::max;  using   std::min;  

	#include	<lvv/math.h>
			using lvv::pow2; using lvv::pow3; using lvv::abs;
	#include	<lvv/array.h>
			using lvv::array;  using lvv::matrix;


	namespace lopti {

	extern "C" void  trsapp_  (int* N, int* NPT, double* XOPT, double* XPT, double* GQ, double* HQ, double* PQ, double* DELTA, double* D, double* W, double* /*W[NP]*/, double* /*W[NP+N]*/, double* /*W[NP+2*N]*/, double* CRVMIN);
	extern "C" void  biglag_  (int* N, int* NPT, double* XOPT, double* XPT, double* BMAT, double* ZMAT, int* IDZ, int* NDIM, int* KNEW, double* DSTEP, double* D, double* ALPHA, double* VLAG, double* /*VLAG[NPT+1]*/, double* W, double* /*W[NP]*/, double* /*W[NP+N]*/);
	extern "C" void  bigden_  (int* N, int* NPT, double* XOPT, double* XPT, double* BMAT, double* ZMAT, int* IDZ, int* NDIM, int* KOPT, int*  KNEW, double* D, double* W, double* VLAG, double* BETA, double* XNEW, double* /*W[NDIM+1]*/, double* /*W[6*NDIM+1]*/);
	extern "C" void  update_  (int* N, int* NPT, double* BMAT, double* ZMAT, int* IDZ, int* NDIM, double* VLAG, double* BETA, int* KNEW, double* W);


		template<typename V, int NPT=2*V::sz+1>
struct  newuoa_minimizer:  trust_region_minimizer<V> {
	//typedef 	typename V::value_type		T;
	typedef 	double				newuoa_t;
	const static int 	N  = V::sz;
	typedef 	array<newuoa_t,N,1>		Vd;

						MINIMIZER_MEMBERS;  TR_MINIMIZER_MEMBERS;  OBJECTIVE_TYPES;
	array<double,N,1>	Xd;	
	//explicit 		newuoa_minimizer	():   trust_region_minimizer<V>("newoua") {};
	minimizer<V>&		x0			(V& _X) 	{  X  = _X;    Xd = _X;  	return *this;  };
	virtual V&		argmin			();
	virtual const string	name			() 	const	{  return minimizer<V>::mk_name("newoua"); };
	//virtual const string	name			() 	const	{  char buf[100];   sprintf(buf,"%s-%d-%d", name_.c_str(),  V::size(),  NPT); return string(buf); };

};

		template<typename V, int NPT> 	V&
newuoa_minimizer<V,NPT>::argmin () {		
						static_assert(V::sz <= NPT, "NPT is too small");		
						assert(!isnan(rho_begin_) && " rho_begin is not definition ");
						assert(!isnan(rho_end_)   && " rho_end   is not definition ");
						///*static*/assert(V::ibg == 1        && " 1st vector index is not == 1 "); //  newuoa have index:  [1:N]
						if (max_iter_ < NPT ) {
							cerr << "newuoa_minimizer warning: max_iter(" << max_iter_ << ") is too small to go beyond initialization phase, continuing anyway." << endl;	
						}

	const static int 	N  = V::sz;
	const int NP = N+1;
	const int NPTM = NPT-NP;
	const int NDIM = NPT+N;

	//if ((NPT < N+2) || ( NPT > ((N+2)*NP)/2))  { cout << "error: NPT should be  N+2 <= NPT  <=  (N+2)*NP)/2  is not in the required interval\n"; exit(33); }
	static_assert(N+2 <= NPT  &&   NPT <= (N+2)*NP/2,  "newuoa error: NPT should be  in  N+2 <= NPT  <=  (N+2)*NP)/2  interval\n");
	if (verbose_) cout << format("newuoa:  N =%d and NPT =%d   ----------------------------------------------------------\n")  % N  % NPT;
	//if (verbose_) printf("newuoa:  N =%d and NPT =%d   ----------------------------------------------------------\n",  N, NPT);

	Vd		XBASE;
	Vd		XOPT; 
	Vd		XNEW;
	Vd		GQ;
	Vd		D;
	lvv::vector<double,NPT>		FVAL;
	lvv::vector<double,NPT>		PQ;
	lvv::vector<double,NDIM>		VLAG;
	lvv::vector<double,(N*NP)/2>		HQ;

	matrix<double,NDIM,N>		BMAT;
	matrix<double,NPT,N>		XPT;  
	matrix<double,NPT,NPTM>		ZMAT;   
		
	array<double,1000,1>		W; //ws

	// lvv: fortran auto declared
	double CRVMIN;
	double DSQ;
	double DSTEP;
	double DISTSQ;
	double ALPHA;
	double BETA;
	double BSUM;
	double DELTA;
	double DETRAT;
	double DIFF;
	double DIFFA;
	double DIFFB;
	double DIFFC;
	double DNORM	= 0;
	double DX;
	double FSAVE;
	double F	= 0;
	double FOPT	= 0;
	double FBEG  	= 0;
	double GQSQ;
	double RATIO 	= 0;
	double RHO;
	double SUM;
	double SUMA;
	double SUMB;
	double SUMZ;
	double TEMP;
	double TEMPQ;
	double VQUAD;
	double XIPT   = 0;
	double XJPT   = 0;
	double XOPTSQ = 0;
	int    IDZ;
	int    IH;
	int    IP;
	int    IPT    = 1;
	int    ITEMP;
	int    ITEST  = 0;
	int    JPT    = 0;
	int    KNEW;
	int    KSAVE;
	int    KTEMP;
	int    NFSAV  = 0;
	int    KOPT   = 0;

	int    _ndim  = NDIM;                		
	int    _n     = N;
	int    _npt   = NPT;

     	double	HALF   = 0.5          ; 		// newuob start
     	double	ONE    = 1.0          ; 
     	double	TENTH  = 0.1          ; 
     	double	ZERO   = 0.0          ; 
     	int	NH     = (N*NP)/2     ; 


	//  Set the initial elements of XPT, BMAT, HQ, PQ and ZMAT to zero.

 	for (int J=1; J<=N; J++)  {
		XBASE[J] = Xd[J];
	 	for (int K=1; K<=NPT; K++)  	XPT(K,J) = ZERO;
	 	for (int I=1; I<=NDIM; I++)  	BMAT(I,J) = ZERO;
	}

 	for (int IH=1; IH<=NH; IH++)  	HQ[IH] = ZERO;

 	for (int K=1; K<=NPT; K++)  {
		PQ[K] = ZERO;
	 	for (int J=1; J<=NPTM; J++)  	ZMAT(K,J) = ZERO;
	}

     	double	RHOSQ = rho_begin_*rho_begin_;
     	double	RECIP = ONE/RHOSQ;
     	double	RECIQ = sqrt(HALF)/RHOSQ;

     	iter_ = 0;

fill_xpt_50:

	//  Begin the initialization procedure. iter_ becomes one more than the number
	//  of function values so far. The coordinates of the displacement of the
	//  next initial interpolation point from XBASE are set in XPT(iter_,.).

	int  NFM = iter_;
     	int  NFMM = iter_ - N;
     	iter_++;
     	if (NFM <= 2*N)  {
     	    if (NFM >= 1  &&  NFM <= N)	XPT(iter_,NFM) = rho_begin_;
     	    else if (NFM > N) 			XPT(iter_,NFMM) = -rho_begin_;
	} else {
     	    ITEMP = (NFMM-1)/N;
     	    JPT = NFM-ITEMP*N-N;
     	    IPT = JPT+ITEMP;
     	    if (IPT > N)  {
     	        ITEMP = JPT;
     	        JPT = IPT-N;
     	        IPT = ITEMP;
     	    }
     	    XIPT = rho_begin_;
     	    if (FVAL[IPT+NP] < FVAL[IPT+1]) XIPT = -XIPT;
     	    XJPT = rho_begin_;
     	    if (FVAL[JPT+NP] < FVAL[JPT+1]) XJPT = -XJPT;
												//assert (0 < iter_ &&  iter_ <= N);
												assert (IPT > 0 &&  IPT <= NPT );
												assert (JPT > 0 &&  JPT <= NPT );
     	    XPT(iter_,IPT) = XIPT;								// gcc warging that IPT, JPT are out of bounds
     	    XPT(iter_,JPT) = XJPT;
     	}

	//  Calculate the next value of F, label 70 being reached immediately
	//  after this calculation. The least function value so far and its index
	//  are required.

 	for (int J=1; J<=N; J++)  	Xd[J] = XPT(iter_,J)+XBASE[J];

     	goto eval_f_310;

return_to_init_from_eval_70:

   	FVAL[iter_] = F;

     	if (iter_ == 1)  {
     	    FBEG = F;
     	    FOPT = F;
     	    KOPT = 1;
     	} else if (F < FOPT)  {
     	    FOPT = F;
     	    KOPT = iter_;
     	}

	//  Set the nonzero initial elements of BMAT and the quadratic model in
	//  the cases when iter_ is at most 2*N+1.

     	if (NFM <= 2*N)  {

     		if (NFM >= 1  &&  NFM <= N)  {
     		    GQ[NFM] = (F-FBEG)/rho_begin_;
     		    if (NPT < iter_+N)  {
     		        BMAT(1,NFM) = -ONE/rho_begin_;
     		        BMAT(iter_,NFM) = ONE/rho_begin_;
     		        BMAT(NPT+NFM,NFM) = -HALF*RHOSQ;
     		    }

     		} else if (NFM > N)  {
     		    BMAT(iter_-N,NFMM) = HALF/rho_begin_;
     		    BMAT(iter_,NFMM) = -HALF/rho_begin_;
     		    ZMAT(1,NFMM) = -RECIQ-RECIQ;
     		    ZMAT(iter_-N,NFMM) = RECIQ;
     		    ZMAT(iter_,NFMM) = RECIQ;
     		    IH = (NFMM*(NFMM+1))/2;
     		    TEMP = (FBEG-F)/rho_begin_;
     		    HQ[IH] = (GQ[NFMM]-TEMP)/rho_begin_;
     		    GQ[NFMM] = HALF*(GQ[NFMM]+TEMP);
     		}

		//  Set the off-diagonal second derivatives of the Lagrange functions and
		//  the initial quadratic model.

     	} else {
     		IH = (IPT*(IPT-1))/2+JPT;
     		if (XIPT < ZERO) IPT = IPT+N;
     		if (XJPT < ZERO) JPT = JPT+N;
     		ZMAT(1,NFMM) = RECIP;
     		ZMAT(iter_,NFMM) = RECIP;
     		ZMAT(IPT+1,NFMM) = -RECIP;
     		ZMAT(JPT+1,NFMM) = -RECIP;
     		HQ[IH] = (FBEG-FVAL[IPT+1]-FVAL[JPT+1]+F)/(XIPT*XJPT);
     	}

     	if (iter_ < NPT) goto fill_xpt_50;

	//  Begin the iterative procedure, because the initial model is complete.

     	RHO = rho_begin_;
     	DELTA = RHO;
     	IDZ = 1;
     	DIFFA = ZERO;
     	DIFFB = ZERO;
     	ITEST = 0;
     	XOPTSQ = ZERO;

 	for (int I=1; I<=N; I++)  {
		XOPT[I] = XPT(KOPT,I);
	   	XOPTSQ = XOPTSQ+pow2(XOPT[I]);
	}

begin_iter_90:

	NFSAV = iter_;

	//  Generate the next trust region step and test its length. Set KNEW
	//  to -1 if the purpose of the next F will be to improve the model.

gen_tr_100:

  	KNEW = 0;
	//trsapp_ (&_n, &_npt, (double*)&XOPT, (double*)&XPT, (double*)&GQ, (double*)&HQ, (double*)&PQ, &DELTA,
	trsapp_ (&_n, &_npt, XOPT.data(), XPT.data(), GQ.data(), HQ.data(), PQ.data(), &DELTA,
		 	D.data(), W.data(), &W[NP], &W[NP+N], &W[NP+2*N], &CRVMIN);  
     	DSQ = ZERO;
 	for (int I=1; I<=N; I++)  DSQ = DSQ+pow2(D[I]);
     	DNORM = min(DELTA,sqrt(DSQ));
     	if (DNORM < HALF*RHO)  {
     	    KNEW = -1;
     	    DELTA = TENTH*DELTA;
     	    RATIO = -1.0;
     	    if (DELTA <= 1.5*RHO) DELTA = RHO;
     	    if (iter_ <= NFSAV+2) goto L460;
     	    TEMP = 0.125*CRVMIN*RHO*RHO;
     	    if (TEMP <= max(DIFFA, max(DIFFB,DIFFC))) goto L460;
     	    goto new_rho_490;
     	}

shift_xbase_120:
	//  Shift XBASE if XOPT may be too far from XBASE. First make the changes
	//  to BMAT that do not depend on ZMAT.

  	if (DSQ <= 1.0 - 3*XOPTSQ)  {
		TEMPQ = 0.25*XOPTSQ;

 		for (int K=1; K<=NPT; K++)  {
			SUM = ZERO;
	 		for (int I=1; I<=N; I++)  	SUM = SUM+XPT(K,I)*XOPT[I];
			TEMP = PQ[K]*SUM;
			SUM = SUM-HALF*XOPTSQ;
			W[NPT+K] = SUM;
	 		for (int I=1; I<=N; I++)  {
				GQ[I] = GQ[I]+TEMP*XPT(K,I);
				XPT(K,I) = XPT(K,I)-HALF*XOPT[I];
				VLAG[I] = BMAT(K,I);
				W[I] = SUM*XPT(K,I)+TEMPQ*XOPT[I];
				IP = NPT+I;
		 		for (int J=1; J<=I; J++)  	BMAT(IP,J) = BMAT(IP,J)+VLAG[I]*W[J]+W[I]*VLAG[J];
		 	}
		}

		//  Then the revisions of BMAT that depend on ZMAT are calculated.

 		for (int K=1; K<=NPTM; K++)  {
			SUMZ = ZERO;

	 		for (int I=1; I<=NPT; I++)  {
				SUMZ = SUMZ+ZMAT(I,K);
		  		W[I] = W[NPT+I]*ZMAT(I,K);
		  	}

	 		for (int J=1; J<=N; J++)  {
				SUM = TEMPQ*SUMZ*XOPT[J];
		 		for (int I=1; I<=NPT; I++)	SUM += W[I]*XPT(I,J);
				VLAG[J] = SUM;
				if (K < IDZ) SUM = -SUM;
		 		for (int I=1; I<=NPT; I++)  	BMAT(I,J) += SUM*ZMAT(I,K);
		 	}

	 		for (int I=1; I<=N; I++)  {
				IP = I+NPT;
				TEMP = VLAG[I];
				if (K < IDZ) TEMP = -TEMP;
		 		for (int J=1; J<=I; J++)  	BMAT(IP,J) = BMAT(IP,J)+TEMP*VLAG[J];
		 	}
	  	}

		//  The following instructions complete the shift of XBASE, including
		//  the changes to the parameters of the quadratic model.

		IH = 0;

 		for (int J=1; J<=N; J++)  {
			W[J] = ZERO;

	 		for (int K=1; K<=NPT; K++)  {
				W[J] = W[J]+PQ[K]*XPT(K,J);
		  		XPT(K,J) -= HALF*XOPT[J];
		  	}

	 		for (int I=1; I<=J; I++)  {
				IH++;
				if (I < J) GQ[J] += HQ[IH]*XOPT[I];
				GQ[I] += HQ[IH]*XOPT[J];
				HQ[IH] += W[I]*XOPT[J]+XOPT[I]*W[J];
		  		BMAT(NPT+I,J) = BMAT(NPT+J,I);
		  	}
		}

 		for (int J=1; J<=N; J++)  {
			XBASE[J] = XBASE[J]+XOPT[J];
	  		XOPT[J] = ZERO;
	  	}

		XOPTSQ = ZERO;
     	}

	//  Pick the model step if KNEW is positive. A different choice of D
	//  may be made later, if the choice of D by BIGLAG causes substantial
	//  cancellation in DENOM.

     	if (KNEW > 0) 
     	    //CALL BIGLAG (N,NPT,XOPT,XPT,BMAT,ZMAT,IDZ,NDIM,KNEW,DSTEP,D,ALPHA,VLAG,VLAG[NPT+1],W,W[NP],W[NP+N])
	    biglag_ (&_n, &_npt, XOPT.data(), XPT.data(), BMAT.data(), ZMAT.data(), &IDZ, &_ndim,
	     &KNEW, &DSTEP, D.data(), &ALPHA, VLAG.data(), &VLAG[NPT+1], W.data(), &W[NP], &W[NP+N]);

	//  Calculate VLAG and BETA for the current choice of D. The first NPT
	//  components of W_check will be held in W.

 	for (int K=1; K<=NPT; K++)  {
		SUMA = ZERO;
		SUMB = ZERO;
		SUM = ZERO;
	 	for (int J=1; J<=N; J++)  {
			SUMA += XPT(K,J)*D[J];
			SUMB += XPT(K,J)*XOPT[J];
		  	SUM  += BMAT(K,J)*D[J];
		}
		W[K] = SUMA*(HALF*SUMA+SUMB);
	  	VLAG[K] = SUM;
	}

     	BETA = ZERO;

 	for (int K=1; K<=NPTM; K++)  {
		SUM = ZERO;
	 	for (int I=1; I<=NPT; I++)  	SUM = SUM+ZMAT(I,K)*W[I];

		if (K < IDZ)  {
		    BETA = BETA+SUM*SUM;
		    SUM = -SUM;
		} else {
		    BETA = BETA-SUM*SUM;
		}
	 	for (int I=1; I<=NPT; I++)  	VLAG[I] += SUM*ZMAT(I,K);
	}

     	BSUM = ZERO;
     	DX = ZERO;

 	for (int J=1; J<=N; J++)  {
		SUM = ZERO;
	 	for (int I=1; I<=NPT; I++)  	SUM = SUM+W[I]*BMAT(I,J);
		BSUM = BSUM+SUM*D[J];
		int JP = NPT+J;
	 	for (int K=1; K<=N; K++)  	SUM = SUM+BMAT(JP,K)*D[K];
		VLAG[JP] = SUM;
		BSUM = BSUM+SUM*D[J];
	  	DX = DX+D[J]*XOPT[J];
	}

     	BETA = DX*DX+DSQ*(XOPTSQ+DX+DX+HALF*DSQ)+BETA-BSUM;
     	VLAG[KOPT] += ONE;

	//  If KNEW is positive and if the cancellation in DENOM is unacceptable,
	//  then BIGDEN calculates an alternative model step, XNEW being used for
	//  working space.

     	if (KNEW > 0)  {
     	    TEMP = ONE+ALPHA*BETA/pow2(VLAG[KNEW]);
     	    if (abs(TEMP) <= 0.8)  {
     	        //CALL BIGDEN (N,NPT,XOPT,XPT,BMAT,ZMAT,IDZ,NDIM,KOPT,KNEW,D,W,VLAG,BETA,XNEW,W[NDIM+1],W[6*NDIM+1])
		bigden_( &_n, &_npt, XOPT.data(), XPT.data(), BMAT.data(), ZMAT.data(), &IDZ, &_ndim, &KOPT,  &KNEW,
		D.data(), W.data(), VLAG.data(), &BETA, XNEW.data(), &W[NDIM+1], &W[6*NDIM+1]);
     	    }
	}
	//  Calculate the next value of the objective function.

 xnew_290:
  	for (int I=1; I<=N; I++) {
		XNEW[I] = XOPT[I]+D[I];
	  	Xd[I] = XBASE[I]+XNEW[I];
	}

     	iter_ = iter_+1;

eval_f_310:

  	if (iter_ > max_iter_)  {
		iter_ = iter_-1;
		if (verbose_) cout << "\n newuoa:  objective function has been called max_iter_ times, terminating, best Xd will be returned";
     	    goto exit_530;
     	}

	X = Xd;		// de-promote

	F = (*this->objective_v)(X);

	if (verbose_) cout << format ("%d \t %18.10g  \t  %18.10g \n")  %iter_  %F  %Xd;
	//if (verbose_) {  printf("%d 	 %18.10g  	  ",  iter_, F);   cout <<  Xd << endl; }

	if (iter_ <= NPT) goto return_to_init_from_eval_70;
	if (KNEW == -1) goto exit_530;

	//  Use the quadratic model to predict the change in F due to the step D,
	//  and set DIFF to the error of this prediction.

     	VQUAD = ZERO;
     	IH = 0;

 	for (int J=1; J<=N; J++)  {
		VQUAD = VQUAD+D[J]*GQ[J];
	 	for (int I=1; I<=J; I++)  {
			IH++;
			TEMP = D[I]*XNEW[J]+D[J]*XOPT[I];
			if (I == J)   TEMP *= HALF;
		  	VQUAD += TEMP*HQ[IH];
		}
	}

 	for (int K=1; K<=NPT; K++)  	VQUAD += PQ[K]*W[K];

     	DIFF = F-FOPT-VQUAD;
     	DIFFC = DIFFB;
     	DIFFB = DIFFA;
     	DIFFA = abs(DIFF);

     	if (DNORM > RHO) NFSAV = iter_;

	//  Update FOPT and XOPT if the new F is the least value of the objective
	//  function so far. The branch when KNEW is positive occurs if D is not
	//  a trust region step.

     	FSAVE = FOPT;

     	if (F < FOPT)  {
     		FOPT = F;
     		XOPTSQ = ZERO;
 		for (int I=1; I<=N; I++)  {
			XOPT[I] = XNEW[I];
		  	XOPTSQ = XOPTSQ+pow2(XOPT[I]);
		}
     	}

     	KSAVE = KNEW;

     	if (KNEW > 0) goto update_410;

	//  Pick the next value of DELTA after a trust region step.

     	if (VQUAD >= ZERO)  {
		if (verbose_)     cout << "\n        error: trust region step has failed to reduce Q.";
		goto exit_530;
     	}

     	RATIO = (F-FSAVE)/VQUAD;

     	if 	(RATIO <= TENTH)	DELTA = HALF*DNORM;
     	else if (RATIO <= 0.7)		DELTA = max(HALF*DELTA,DNORM);
     	else				DELTA = max(HALF*DELTA,DNORM+DNORM);

     	if (DELTA <= 1.5*RHO)		DELTA = RHO;

	//  Set KNEW to the index of the next interpolation point to be deleted.

     	RHOSQ = pow2(max(TENTH*DELTA,RHO));
     	KTEMP = 0;
     	DETRAT = ZERO;

     	if (F >= FSAVE) { 
		KTEMP = KOPT;
		DETRAT = ONE;
     	}

 	for (int K=1; K<=NPT; K++)  {

		double HDIAG = ZERO;

	 	for (int J=1; J<=NPTM; J++)  {
			TEMP = ONE;
			if (J < IDZ) TEMP = -ONE;
		  	HDIAG = HDIAG+TEMP*pow2(ZMAT(K,J));
		}

		TEMP = abs(BETA*HDIAG+pow2(VLAG[K]));
		DISTSQ = ZERO;

	 	for (int J=1; J<=N; J++)  	DISTSQ = DISTSQ+pow2((XPT(K,J)-XOPT[J]));

		if (DISTSQ > RHOSQ)		TEMP = TEMP*pow3(DISTSQ/RHOSQ);

		if (TEMP > DETRAT  &&  K != KTEMP)  {
		    DETRAT = TEMP;
		    KNEW = K;
		}
	}

     	if (KNEW == 0) goto L460;

	//  Update BMAT, ZMAT and IDZ, so that the KNEW-th interpolation point
	//  can be moved. Begin the updating of the quadratic model, starting
	//  with the explicit second derivative term.

update_410:

	// CALL UPDATE (N,NPT,BMAT,ZMAT,IDZ,NDIM,VLAG,BETA,KNEW,W)
	//update_  (&_n, &_npt, (double*)&BMAT, (double*)&ZMAT, &IDZ, &_ndim, (double*)&VLAG, &BETA, &KNEW, (double*)&W);
	update_  (&_n, &_npt, BMAT.data(), ZMAT.data(), &IDZ, &_ndim, VLAG.data(), &BETA, &KNEW, W.data());

     	FVAL[KNEW] = F;
     	IH = 0;

 	for (int I=1; I<=N; I++)  {
		TEMP = PQ[KNEW]*XPT(KNEW,I);
	 	for (int J=1; J<=I; J++)  {
			IH = IH+1;
		  	HQ[IH] = HQ[IH]+TEMP*XPT(KNEW,J);
		}
	}

     	PQ[KNEW] = ZERO;

	//  Update the other second derivative parameters, and then the gradient
	//  vector of the model. Also include the new interpolation point.

 	for (int J=1; J<=NPTM; J++)  {
		TEMP = DIFF*ZMAT(KNEW,J);
		if (J < IDZ) 			TEMP = -TEMP;
	 	for (int K=1; K<=NPT; K++)  	PQ[K] = PQ[K]+TEMP*ZMAT(K,J);
	}

     	GQSQ = ZERO;
 	for (int I=1; I<=N; I++)  {
		GQ[I] = GQ[I]+DIFF*BMAT(KNEW,I);
		GQSQ = GQSQ+pow2(GQ[I]);
	  	XPT(KNEW,I) = XNEW[I];
	}

	//  If a trust region step makes a small change to the objective function,
	//  then calculate the gradient of the least Frobenius norm interpolant at
	//  XBASE, and store it in W, using VLAG for a vector of right hand sides.

     	if (KSAVE == 0  &&  DELTA == RHO)  {
     		if (abs(RATIO) > 0.01)  {
     			ITEST = 0;
     		} else {
			for (int K=1; K<=NPT; K++)  	VLAG[K] = FVAL[K]-FVAL[KOPT];
     			double GISQ = ZERO;

 			for (int I=1; I<=N; I++)  {
				SUM = ZERO;
	 			for (int K=1; K<=NPT; K++)  	SUM += BMAT(K,I)*VLAG[K];
				GISQ = GISQ+SUM*SUM;
	  			W[I] = SUM;
	  		}

			//  Test whether to replace the new quadratic model by the least Frobenius
			//  norm interpolant, making the replacement if the test is satisfied.

     			ITEST = ITEST+1;
     			if (GQSQ < 100*GISQ) ITEST = 0;

     			if (ITEST >= 3)  {
 				for (int I=1; I<=N; I++)  	GQ[I] = W[I];
 				for (int IH=1; IH<=NH; IH++)  	HQ[IH] = ZERO;

 				for (int J=1; J<=NPTM; J++)  {
					W[J] = ZERO;
	 				for (int K=1; K<=NPT; K++)  	W[J] = W[J]+VLAG[K]*ZMAT(K,J);
	  				if (J < IDZ)			W[J] = -W[J];
	  			}

 				for (int K=1; K<=NPT; K++)  {
					PQ[K] = ZERO;
	 				for (int J=1; J<=NPTM; J++)  	PQ[K] = PQ[K]+ZMAT(K,J)*W[J];
	  			}

				ITEST = 0;
     			}
     		}
     	}

     	if (F < FSAVE)		KOPT = KNEW;

	//  If a trust region step has provided a sufficient decrease in F, then
	//  branch for another trust region calculation. The case KSAVE>0 occurs
	//  when the new function value was calculated by a model step.

     	if (F <= FSAVE+TENTH*VQUAD)	goto gen_tr_100;
     	if (KSAVE > 0)			goto gen_tr_100;

	//  Alternatively, find out if the interpolation points are close enough
	//  to the best point so far.

     	KNEW = 0;

  L460:

  	DISTSQ = 4.0*DELTA*DELTA;

 	for (int K=1; K<=NPT; K++)  {
		SUM = ZERO;
	 	for (int J=1; J<=N; J++)  	SUM += pow2(XPT(K,J)-XOPT[J]);

		if (SUM > DISTSQ)  { 
		    KNEW = K;
		    DISTSQ = SUM;
		}
	}

	//  If KNEW is positive, then set DSTEP, and branch back for the next
	//  iteration, which will generate a "model step".

     	if (KNEW > 0)  {
     	    DSTEP = max(min(TENTH*sqrt(DISTSQ),HALF*DELTA),RHO);
     	    DSQ = DSTEP*DSTEP;
     	    goto shift_xbase_120;
     	}

     	if (RATIO > ZERO)		goto gen_tr_100;
     	if (max(DELTA,DNORM) > RHO)	goto gen_tr_100;


new_rho_490: 

	//  The calculations with the current value of RHO are complete. Pick the
	//  next values of RHO and DELTA.

  	if (RHO > rho_end_)  { 

		DELTA = HALF*RHO;
		RATIO = RHO/rho_end_;
		
		if	(RATIO <= 16.0)	RHO = rho_end_;
		else if	(RATIO <= 250.0)	RHO = sqrt(RATIO)*rho_end_;
		else				RHO = TENTH*RHO;
		
		DELTA = max(DELTA,RHO);

		if (verbose_) cout << format("-- (%d) RHO =%9.6g \t F =%9.6g   Xd%.8g \n")   %iter_  %RHO  %FOPT  %Xd;
		//if (verbose_) printf("-- (%d) RHO =%9.6g 	 F =%9.6g   Xd: ", iter_,  RHO,  FOPT);  cout << Xd << endl;
		goto  begin_iter_90; 
     	}

	//  Return from the calculation, after another Newton-Raphson step, if
	//  it is too short to have been tried before.

     	if (KNEW == -1)		goto xnew_290;

  exit_530:

	if (FOPT <= F)  {
	 	for (int I=1; I<=N; I++)  	Xd[I] = XBASE[I]+XOPT[I];
		F = FOPT;
	}

	if (verbose_)  cout << format("-- (%d) RETURNED: \t F =%.15g    Xd is: %.15g\n\n")  %iter_ %F  %Xd;
	//if (verbose_)  { printf("-- (%d) RETURNED: 	 F =%.15g    Xd is: ",  iter_,  F);   cout << Xd << endl << endl; }

	ymin_ = F;
	Xmin_ = Xd;
	return Xmin_;
}; // newuoa

	} // namespace lopti
	#endif // LOPTI_NEWUOA_H
