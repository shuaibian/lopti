
	#ifndef LOPTI_NELDER_MEAD_H
	#define LOPTI_NELDER_MEAD_H

	#ifndef		MINIMIZER
		#define	MINIMIZER	gsl_nelder_mead_minimizer
	#endif

	#ifndef		V_IBEGIN
		#define	V_IBEGIN 0
	#endif
	

	#include <lopti/convert-gsl.h>
		using lvv::array;
	#include <lopti/lopti.h>
	#include <stdlib.h>
	#include <gsl/gsl_errno.h>
	#include <gsl/gsl_math.h>
	#include <gsl/gsl_multimin.h>

	#include <lvv/lvv.h>
		using std::cerr;

	
	namespace lopti {
 
 template<typename V>		struct gsl_of_wrap {
				OBJECTIVE_TYPES;
				static		objective_p_t	 objective_v;
		static void	init	(objective_p_t  _objective_v)			{ objective_v = _objective_v; }	
		static double	eval	(const gsl_vector* gX, void * var)	{ V X;    X << gX;    return  (*objective_v)(X); }	
 };

//template<typename V>  objective0<V>* gsl_of_wrap<V>::objective_v; // this is in gsl_of_wrap class, but we need to decl it 1 more time for compiler
template<typename V>  shared_ptr<objective0<V>> gsl_of_wrap<V>::objective_v; // this is in gsl_of_wrap class, but we need to decl it 1 more time for compiler

                 template<typename V>
struct	gsl_nelder_mead_minimizer  :  minimizer<V> {
						MINIMIZER_MEMBERS;   OBJECTIVE_TYPES;
		gsl_vector*			gX;	
		gsl_vector* 			gS;	
		const gsl_multimin_fminimizer_type *gsl_minimizer_type_ptr;
		gsl_multimin_fminimizer*	gsl_minimizer;
		gsl_multimin_function		minex_func;
		double 				characteristic_size_;

	const string		name			() 	const	{  return minimizer<V>::mk_name("nelder-meed"); };
	explicit 		gsl_nelder_mead_minimizer	() :
			//minimizer<V>("nelder-mead"),
			gsl_minimizer_type_ptr(gsl_multimin_fminimizer_nmsimplex),
			gX(0),
			gS(0)
		{};

	~gsl_nelder_mead_minimizer () { gsl_multimin_fminimizer_free(gsl_minimizer);  gsl_vector_free(gX);   gsl_vector_free(gS);  };

	virtual minimizer<V>&		step0			(V& S)		{
		gS = gsl_vector_alloc(V::size()); 
		gS << S;  // convert to gsl_vector
		gsl_minimizer = gsl_multimin_fminimizer_alloc(gsl_minimizer_type_ptr, V::size());
		if (verbose_)   cerr << "#  minimizer: "  <<  gsl_multimin_fminimizer_name (gsl_minimizer)  <<  endl;
		return *this;
	};

	//void 		gsl_var			(void* var )	{ minex_func.params = var; };
	virtual  minimizer<V>&		characteristic_size	(T cs)	{ characteristic_size_ = cs;  return *this; };

	virtual V&		argmin () {
		
		//// set defaults if parameters are missing
		if (gS == 0)		{ V S;	S = 0.2;	step0(S); }
		//if (gX == 0) 		{ V X;	X = 0;		x0(X); }

		////  gsl init
		gsl_of_wrap<V>::init(objective_v);
		minex_func.f = &gsl_of_wrap<V>::eval;
		minex_func.n = X.size();
		//minex_func.params = var;
		minex_func.params = NULL;
		gX  = gsl_vector_alloc(X.size());
		gX << X;
		gsl_multimin_fminimizer_set(gsl_minimizer, &minex_func, gX  , gS);

		///// main cicle
		int	test_status=GSL_CONTINUE;			// test_status:  GSL_SUCCESS==0; GSL_CONTINUE==-2; 

		//if (verbose_)  FMT("# Itr  %10t Y   %20t  X[0..]   Step\n");
		if (verbose_)  cout << "# Itr  %10t Y   %20t  X[0..]   Step\n";
	
		for  ( iter_=0;  iter_ < max_iter_  &&  (test_status==GSL_CONTINUE);   ++iter_ )   {

			int  iterate_status = gsl_multimin_fminimizer_iterate(gsl_minimizer);

			if (iterate_status) {
				cerr << "error: FMinimizer: in gsl_multimin_fminimizer_iterate()\n";
				break;
			}

			double size = gsl_multimin_fminimizer_size(gsl_minimizer);
			test_status = gsl_multimin_test_size(size, characteristic_size_);

			if (test_status == GSL_SUCCESS)  {
				found_=true;
				//fmin=gsl_minimizer->fval;
				if (verbose_ )    cerr  << "# converged to minimum at\n";
			}
			
			if (verbose_) { 
				printf("%5d 	 %18.10g 	 ", iter_,  gsl_minimizer->fval);
				for (int i=0; i < gsl_minimizer->x->size; ++i) printf("%18.10g",  gsl_vector_get(gsl_minimizer->x, i));
				printf("%18.10g", size);
				cout << endl;
			}
		}

		ymin_ = gsl_minimizer->fval;
		Xmin_  <<  gsl_minimizer->x;
		return Xmin_;
	}

 };
	}	// namespace  lopti
	#endif // LOPTI_NELDER_MEAD_H
