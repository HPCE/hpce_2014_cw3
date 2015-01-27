#include "fourier_transform.hpp"

#include <cstdlib>
#include <random>
#include <iostream>
#include <string>

using namespace hpce;

///////////////////////////////////////////////////////////////////////////////
// Very simple testing infrastructure

static std::mt19937 s_urng;

static unsigned randomUniformInt(unsigned n)
{
	std::uniform_int_distribution<unsigned> dst(0,n);
	return dst(s_urng);
}

static double randomUniformReal()
{
	std::uniform_real_distribution<double> dst;
	return dst(s_urng);
}

static complex_t randomUniformComplex()
{
	double re=randomUniformReal();
	double im=randomUniformReal();
	return complex_t(re, im);
}

static std::string s_currentTest;
static unsigned s_totalFailed=0;
static unsigned s_totalPassed=0;

template<class T>
static void verifyEqual(const T&got, const T &want, const std::string &message)
{
	if(got!=want){
		std::cerr<<"FAIL: "<<s_currentTest<<" got="<<got<<", expected="<<want<<", "<<message<<std::endl;
		s_totalFailed++;
	}
	s_totalPassed++;
}

template<class T>
static void verifyClose(const T&got, const T &want, double absTol, const std::string &message)
{
	if(std::abs(got-want) > absTol){
		std::cerr<<"FAIL: "<<s_currentTest<<" got="<<got<<", expected="<<want<<" (difference="<<got-want<<"), "<<message<<std::endl;
		s_totalFailed++;
	}
	s_totalPassed++;
}


//////////////////////////////////////////////////////////////////////////////////
// Individual tests to apply to a transform

static void testDCTransform(const fourier_transform *pTransform, size_t n)
{
	std::stringstream acc;
	acc<<"testDCTransform("<<pTransform->name()<<","<<n<<")";
	s_currentTest=acc.str();
	
	complex_vec_t input(n, complex_t(1.0, 0.0));
	complex_vec_t forward = pTransform->forwards(input);
	
	verifyEqual(forward[0], (complex_t)(double)n, "DC component is not n.");
	
	if(forward.size()==n){
		// As long as the array has not been padded, all the other components should be zero
		for(unsigned i=1;i<n;i++){
			verifyClose(forward[i], complex_t(0.0), 1e-9, "AC component is non-zero.");
		}
	}
	
	complex_vec_t backward = pTransform->backwards(forward, n);
	
	verifyEqual(backward.size(), n, "Size of output does not match.");
	for(unsigned i=1;i<n;i++){
		// TODO : change to verifyClose
		verifyClose(backward[i], input[i], 1e-9, "backwards(forwards(vec))!=vec .");
	}
}


static void testUniformRandom(const fourier_transform *pTransform, size_t n)
{
	std::stringstream acc;
	acc<<"testDCTransform("<<pTransform->name()<<","<<n<<")";
	s_currentTest=acc.str();
	
	complex_vec_t input(n, 1.0);
	for(unsigned i=0;i<n;i++){
		input[i]=randomUniformComplex();
	}
	
	complex_vec_t output = pTransform->backwards( pTransform->forwards(input) );
	
	for(unsigned i=0;i<n;i++){
		verifyClose(output[i], input[i], 1e-9, " Non reversible transform.");
	}
}

////////////////////////////////////////////////////////////////////////////////////
// Basic driver to start things up

int main(int argc, char *argv[])
{
	try{
		// Put the system in a known state
		s_urng=std::mt19937(1);
		
		fourier_transform::RegisterDefaultFactories();
		
		if(argc<=1){
			auto names = fourier_transform::GetTransformFactoryNames();
			auto it=names.begin();
			while(it!=names.end()){
				std::cerr<<*it<<std::endl;
				++it;
			}
			return 1;
		}
		
		std::string name(argv[1]);
		std::shared_ptr<fourier_transform> transform=fourier_transform::CreateTransform(name);
		
		
		/////////////////////////////////////////////////////////////////////////
		// Begin the tests
		
		// Check the forward and backwards transform of pure DC.
		for(unsigned i=1;i<=13;i++){
			testDCTransform(transform.get(), 1u<<i);
		}
		if(!transform->is_quadratic()){
			for(unsigned i=14;i<=24;i++){
				testDCTransform(transform.get(), 1u<<i);
			}
		}
		
		// Generate random vectors
		for(unsigned i=2;i<100;i++){
			testUniformRandom(transform.get(), i);
		}
		if(!transform->is_quadratic()){
			for(unsigned i=0;i<10;i++){
				testUniformRandom(transform.get(), randomUniformInt(1000000)+10);
			}
		}
		
		std::cout<<transform->name()<<" : Passed "<<s_totalPassed<<" out of "<<(s_totalFailed+s_totalPassed)<<" tests."<<std::endl;
		
		return s_totalFailed>0 ? 1 : 0;
	}catch(std::exception &e){
		std::cerr<<"Caught exception: "<<e.what()<<"\n";
		return 1;
	}catch(...){
		std::cerr<<"Caught unexpected exception type.\n";
		return 1;
	}
}
