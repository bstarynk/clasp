       
       
//
// (C) 2004 Christian E. Schafmeister
//



#ifndef	PYTHONCALLBACK_H
#define	PYTHONCALLBACK_H



//
//
//	Dumb_PythonCallback
//
//	Malongains a callback to a Python function.
//	Also malongains two doubles and two longegers that can be passed
//	back and forth between the Python function and the
//	caller to communicate progress to Python and to tell
//	the caller that it should terminate.
//
//
//	The Python callback function should accept two doubles and two longegers.
//	It should return an longeger.
//	def callback( d0, d1, long0, long1):
//		return long
//
//

#include "foundation.h"


#ifdef	USEBOOSTPYTHON
#include <Python.h>
#endif


namespace core {


class	Dumb_PythonCallback;
    typedef	gctools::smart_ptr<Dumb_PythonCallback>	RPPythonCallback;
class	Dumb_PythonCallback{

public:
    	friend	RPPythonCallback	PythonCallback();

#ifdef	USEBOOSTPYTHON
	void	setCallback(PyObject*	callback);
#endif

	void	callCallback();

	void	setDoubleVal0(double v) { this->doubleVal0 = v;};
	double	getDoubleVal0()		{ return this->doubleVal0; };
	void	setDoubleVal1(double v) { this->doubleVal1 = v;};
	double	getDoubleVal1()		{ return this->doubleVal1; };
	void	setLongVal0(long v) 	{ this->longVal0 = v;};
	long	getLongVal0() 		{ return this->longVal0;};
	void	setLongVal1(long v) 	{ this->longVal1 = v;};
	long	getLongVal1() 		{ return this->longVal0;};
	long	getLongReturn() 	{ return this->longReturn;};

#ifdef	USEBOOSTPYTHON
	bool	callbackIsActive()	{ return (this->callbackFn!=NULL); };
#else
	bool	callbackIsActive()	{ return false; };
#endif


private:
	double		doubleVal0;
	double		doubleVal1;
	long		longVal0;
	long		longVal1;
	long		longReturn;

#ifdef	USEBOOSTPYTHON
	PyObject*	callbackFn;
#endif


};

inline RPPythonCallback	PythonCallback() { return RPPythonCallback(new Dumb_PythonCallback());};


};
#endif
