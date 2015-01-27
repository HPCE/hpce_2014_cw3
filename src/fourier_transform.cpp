#include "fourier_transform.hpp"

#include <map>

#include <iostream>

namespace hpce
{

static std::map<std::string,fourier_transform_factory_t> s_factories;

std::complex<double>  fourier_transform::root_of_unity(size_t n) const
{
	const double pi2=6.283185307179586476925286766559;
	double angle = pi2/n;
	return complex_t(cos(angle), sin(angle));
}
	

complex_vec_t fourier_transform::forwards(const complex_vec_t &in) const
{
	
	size_t n=calc_padded_size(in.size());
	complex_vec_t result(n);
	
	complex_t wn=root_of_unity(n);
	
	if(n==in.size()){
		forwards_impl(n, wn, &in[0], 1, &result[0], 1);
	}else{
		complex_vec_t buffer(in);
		buffer.resize(n, complex_t(0.0,0.0) );
		forwards_impl(n, wn, &buffer[0], 1, &result[0], 1);
	}
	
	return result;
}
	
complex_vec_t fourier_transform::backwards(const complex_vec_t &in, size_t n) const
{
	if(n==0){
		n=in.size();
	}else{
		if(n>in.size())
			throw std::invalid_argument("Output size must be less than or equal to input size.");
	}
		
	complex_vec_t result(in.size());
	
	complex_t wn=root_of_unity(n);
	backwards_impl(n, wn, &in[0], 1, &result[0], 1);
	
	result.resize(n);
	return result;
}

/*! Register a fourier transform factory by name. */
void fourier_transform::RegisterTransformFactory(std::string name, fourier_transform_factory_t factory)
{
	auto it=s_factories.find(name);
	if(it!=s_factories.end())
		throw std::invalid_argument("Factory already registered with that name.");
	
	s_factories.insert(std::make_pair(name, factory));
}

/*! List the set of transform factory names */
std::list<std::string> fourier_transform::GetTransformFactoryNames()
{
	std::list<std::string> names;
	auto it=s_factories.begin();
	while(it!=s_factories.end()){
		names.push_back(it->first);
		++it;
	}
	
	return names;
}

/*! Create an instance of a fourier transform */
std::shared_ptr<fourier_transform> fourier_transform::CreateTransform(const std::string &name)
{
	auto it=s_factories.find(name);
	if(it==s_factories.end())
		throw std::invalid_argument("No factory registered with that name.");
	
	return it->second();
}

}; // namespace hpce
