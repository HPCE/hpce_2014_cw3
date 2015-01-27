#include "fourier_transform.hpp"

#include <cstdlib>
#include <random>
#include <iostream>
#include <string>
#include <algorithm>
#include <numeric>

#include "tbb/tick_count.h"
#include "tbb/task_scheduler_init.h"

using namespace hpce;

///////////////////////////////////////////////////////////////////////////////
// Very simple testing infrastructure

static std::mt19937 s_urng;

static double randomUniformReal()
{
	std::uniform_real_distribution<double> dst(-1.0, 1.0);
	return dst(s_urng);
}

static complex_t randomUniformComplex()
{
	double re=randomUniformReal();
	double im=randomUniformReal();
	return complex_t(re, im);
}


int main(int argc, char *argv[])
{
	try{
		// Put the system in a known state
		s_urng=std::mt19937(1);
		
		fourier_transform::RegisterDefaultFactories();
		
		/////////////////////////////////////////////////////////////////
		// Handle command line arguments
		
		if(argc<2){
			std::cerr<<"time_fourier_transform name [P] [maxTime]"<<"\n";
			std::cerr<<"    Test the performance of the named fourier transform with P processors."<<"\n";
			std::cerr<<"    P : Number of processors to allow TBB to use.\n";
			std::cerr<<"    maxTime : Maximum time to allow any single test to run for.\n";
			std::cerr<<"\n    Implementations:\n";
		
			auto names = fourier_transform::GetTransformFactoryNames();
			auto it=names.begin();
			while(it!=names.end()){
				std::cerr<<"        "<<*it<<std::endl;
				++it;
			}
			return 1;
		}
		
		std::string name(argv[1]);
		std::shared_ptr<fourier_transform> transform=fourier_transform::CreateTransform(name);
		
		unsigned allowedP=0;
		if(argc>2){
			allowedP=atoi(argv[2]);
		}
		
		double maxTime=5;
		if(argc>3){
			maxTime=strtod(argv[3], NULL);
		}
		
		//////////////////////////////////////////////////////////
		// Now do timing for increasing sizes		
		
		// This is the default parallelism detected in the machine by TBB
		unsigned trueP=tbb::task_scheduler_init::default_num_threads();
		
		if(allowedP==0)
			allowedP=trueP;
		
		// This is the number of parallel tasks that TBB will support
		tbb::task_scheduler_init task_init(allowedP);
		
		double log2n=4;
		
		std::cout<<"# name, allowedP, trueP, n, [sentinel], time\n";
		
		while(log2n <= 26){	// Try not to blow up memory system
			size_t n=(size_t)std::pow(2.0, log2n);
			
			// Set up random numbers between -1 and +1
			complex_vec_t input(n);
			for(size_t i=0;i<n;i++){
				input[i]=randomUniformComplex();
			}
			
			tbb::tick_count t_start = tbb::tick_count::now();
			
			// A single forwards and backwards transform
			input = transform->backwards(transform->forwards(input), n);
			
			tbb::tick_count t_finish = tbb::tick_count::now();
			
			double time=(t_finish-t_start).seconds();
			
			// This is to make absolutely sure that the compiler cannot optimise away any of
			// the calculations. It is over the top, but ensures that all calculations are really
			// done.
			complex_t sentinel=std::accumulate(input.begin(), input.end(), complex_t(0.0));
			
			std::cout<<transform->name()<<", "<<allowedP<<", "<<trueP<<", "<<n<<", "<<std::abs(sentinel)<<", "<<time<<"\n";
			
			if(time >= maxTime){
				break;
			}
			
			log2n+=0.25;
		}
		
		return 0;
	}catch(std::exception &e){
		std::cerr<<"Caught exception: "<<e.what()<<"\n";
		return 1;
	}catch(...){
		std::cerr<<"Caught unexpected exception type.\n";
		return 1;
	}
}
