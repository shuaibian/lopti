

#include  <iostream>
	using namespace std;

#include <lvv/lvv.h>
//	using namespace lvv;

	#include <lvv/array.h>
		using lvv::array;

	#include  <functional>
		using std::unary_function;

#include  "lopti.h"
	using namespace lopti;

#include  "line_search.h"


int main() { 

		typedef 	array<double,2,1>		V;	
		//line_search_backtracking_t<V>	ls = line_search_backtracking_t<V>(trace<V>(rosenbrock<V>()));
		line_search_backtracking_t<V>	ls = line_search_backtracking_t<V>(trace<V>(rosenbrock<V>()));
		const V		X0 = {{ 1, 0 }};
		const V		DX = {{ 0, 1 }};

	cout << "starting point:    " << X0 << endl;
	cout << "direction:         " << DX << endl;
	//cout << "line search found: " << ls.find (X0, DX) << endl;
		V		RES(ls.find (X0, DX));
	cout << "line search found: " << RES << endl;
	
	cout << endl;
}
