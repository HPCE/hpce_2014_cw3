#include "fourier_transform.hpp"

#include <cmath>
#include <cassert>

namespace hpce
{

class direct_fourier_transform
	: public fourier_transform
{
protected:
	/*! We can do any size transform */
	virtual size_t calc_padded_size(size_t n) const
	{
		return n;
	}

	virtual void forwards_impl(
		size_t n,	const std::complex<double> &/*wn*/,
		const std::complex<double> *pIn, size_t sIn,
		std::complex<double> *pOut, size_t sOut
	) const 
	{
		assert(n>0);
		
		const double PI=3.1415926535897932384626433832795;
		
		// = -i*2*pi / n
		complex_t neg_im_2pi_n=-complex_t(0.0, 1.0)*2.0*PI / (double)n;
		
		for(size_t kk=0;kk<n;kk++){
			complex_t acc=0;
			for(size_t ii=0;ii<n;ii++){
				// acc += exp(-i * 2 * pi * kk / n);
				acc+=pIn[ii*sIn] * exp( neg_im_2pi_n * (double)kk * (double)ii );
			}
			pOut[kk*sOut]=acc;
		}
	}
	
	virtual void backwards_impl(
		size_t n,	const std::complex<double> &/*wn*/,
		const std::complex<double> *pIn, size_t sIn,
		std::complex<double> *pOut, size_t sOut
	) const 
	{
		assert(n>0);
		
		const double PI=3.1415926535897932384626433832795;
		
		// = i*2*pi / n
		complex_t im_2pi_n=complex_t(0.0, 1.0)*2.0*PI / (double)n;
		
		const double scale=1.0/n;
		
		for(size_t kk=0;kk<n;kk++){
			complex_t acc=0;
			for(size_t ii=0;ii<n;ii++){
				// acc += exp(i * 2 * pi * kk / n);
				acc+=pIn[ii*sIn] * exp( im_2pi_n * (double)kk * (double)ii );
			}
			pOut[kk*sOut]=acc*scale;
		}
	}
	
public:
	virtual std::string name() const
	{ return "hpce.direct_fourier_transform"; }
	
	virtual bool is_quadratic() const
	{ return true; }
};

std::shared_ptr<fourier_transform> Create_direct_fourier_transform()
{
	return std::make_shared<direct_fourier_transform>();
}

}; // namespace hpce
