#ifndef fourier_transform_hpp
#define fourier_transform_hpp

#include <vector>
#include <list>
#include <complex>
#include <functional>
#include <memory>

namespace hpce
{
	
class fourier_transform;

typedef std::complex<double> complex_t;
typedef std::vector<std::complex<double> > complex_vec_t;
	
//! The type of a function which knows how to create fourier transform instances
typedef std::function< std::shared_ptr<fourier_transform>() > fourier_transform_factory_t;

class fourier_transform
{
protected:	
	std::complex<double> root_of_unity(size_t n) const;

	virtual size_t calc_padded_size(size_t n) const=0;

	virtual void forwards_impl(
		size_t n,	const complex_t &wn,
		const complex_t *pIn, size_t sIn,
		complex_t *pOut, size_t sOut
	) const =0;
	
	virtual void backwards_impl(
		size_t n,	const complex_t &wn,
		const complex_t *pIn, size_t sIn,
		complex_t *pOut, size_t sOut
	) const =0;
	
public:
	virtual ~fourier_transform()
	{}
	
	/*! Identifier for this transform. */
	virtual std::string name() const =0;
		
	/*! Return true if the algorithm takes quadratic time (rather than n log n) */
	virtual bool is_quadratic() const =0;
	
	/*! Does a forward FFT.
		\param in Takes in a an array of any size and complex-ness
		\retval The fourier transform of the array.
		\note If necessary this will pad the input up to a convenient boundary, so the
			output may be bigger than the input.
	*/
	complex_vec_t forwards(const complex_vec_t &in) const ;
	
	/*! Does a backward FFT.
		\param in Takes in a an array, which must be of a size compatible with this fourier transform
		\param n The number of elements to return - must have n<=in.size()
		\retval The un-fourier matrix of length n
	*/
	complex_vec_t backwards(const complex_vec_t &in, size_t n=0) const;
	
	/*! Register a fourier transform factory by name. */
	static void RegisterTransformFactory(std::string name, fourier_transform_factory_t factory);
	
	/*! List the set of transform factory names 	*/
	static std::list<std::string> GetTransformFactoryNames();
	
	/*! Create an instance of a fourier transform */
	static std::shared_ptr<fourier_transform> CreateTransform(const std::string &name);
	
	/*! Used to add all factories. known at run-time */
	static void RegisterDefaultFactories();
};

};

#endif
