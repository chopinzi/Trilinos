// @HEADER
// ************************************************************************
//
//               Rapid Optimization Library (ROL) Package
//                 Copyright (2014) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact lead developers:
//              Drew Kouri   (dpkouri@sandia.gov) and
//              Denis Ridzal (dridzal@sandia.gov)
//
// ************************************************************************
// @HEADER

/*! \file  materials.hpp
*/

#ifndef MATERIALS_HPP
#define MATERIALS_HPP

#include "Intrepid_FieldContainer.hpp"

/** \class  Materials
*/

template <class Real>
class Material {

protected:

int dim_;
int tensorMatSize_;

bool planeStrain_;
Real modulus_;
Real poissonRatio_;
Teuchos::RCP<Intrepid::FieldContainer<Real> > materialTensor_;

public:
  
Material() {}
virtual ~Material() {}

virtual void InitializeMaterial(int dim, bool planeStrain, Real modulus, Real poissonRatio)
{
	dim_ = dim;
	planeStrain_ = planeStrain;
	modulus_ = modulus;
	poissonRatio_ = poissonRatio; 
	if(dim_==1)
		tensorMatSize_ = 1;
	else if(dim_==2)
		tensorMatSize_ = 3;
	else
		tensorMatSize_ = 6;
	
	ComputeMaterialTensor();
}

void ComputeMaterialTensor()
{
	materialTensor_ = Teuchos::rcp(new Intrepid::FieldContainer<Real>(tensorMatSize_, tensorMatSize_));
	materialTensor_ -> initialize(0.0);
	if(dim_==1)
		(*materialTensor_)(0, 0)=modulus_;
	else if(dim_==2 && !planeStrain_)
	{
		Real factor1 = modulus_ /(1-poissonRatio_*poissonRatio_);
		(*materialTensor_)(0, 0) = factor1;
		(*materialTensor_)(0, 1) = factor1 * poissonRatio_;
		(*materialTensor_)(1, 0) = factor1 * poissonRatio_;
		(*materialTensor_)(1, 1) = factor1;
		(*materialTensor_)(2, 2) = factor1 * (1-poissonRatio_)/2;
		
	}		
	else if(dim_==2 && planeStrain_)
	{
		Real factor2 = modulus_ /(1+poissonRatio_)/(1-2*poissonRatio_);
		(*materialTensor_)(0, 0) = factor2 * (1-poissonRatio_);
		(*materialTensor_)(0, 1) = factor2 * poissonRatio_;
		(*materialTensor_)(1, 0) = factor2 * poissonRatio_;
		(*materialTensor_)(1, 1) = factor2 * (1-poissonRatio_);
		(*materialTensor_)(2, 2) = factor2 * (1-2*poissonRatio_)/2;
	}
	else
	{
		Real lam = modulus_*poissonRatio_/(1+poissonRatio_)/(1-2*poissonRatio_);
		Real mu = modulus_/2/(1+poissonRatio_);
		(*materialTensor_)(0, 0)=lam + 2*mu;
		(*materialTensor_)(0, 1)=lam;
		(*materialTensor_)(0, 2)=lam;
		(*materialTensor_)(1, 0)=lam;
		(*materialTensor_)(1, 1)=lam + 2*mu;
		(*materialTensor_)(1, 2)=lam;
		(*materialTensor_)(2, 0)=lam;
		(*materialTensor_)(2, 1)=lam;
		(*materialTensor_)(2, 2)=lam + 2*mu;
		(*materialTensor_)(3, 3)=mu;
		(*materialTensor_)(4, 4)=mu;
		(*materialTensor_)(5, 5)=mu;
	}				
}

Teuchos::RCP<Intrepid::FieldContainer<Real> > GetMaterialTensor() const
{
	return materialTensor_;
}

int GetMaterialTensorDim() const
{
	return tensorMatSize_;
}

Real GetYoungsModulus() const
{
	return modulus_;
}

Real GetPoissonRatio() const
{
	return poissonRatio_;
}

Real GetBulkModulus() const { }

Real GetShearModulus() const
{
	return modulus_/2/(1+poissonRatio_); 
}

void CheckMaterialTensor()
{
	std::cout<< *materialTensor_ <<std::endl;
}

};


// new class for topology optimization
template<class Real>
class Material_SIMP : public Material <Real> {

private:

Real density_;
int powerP_;

public:

virtual void InitializeSIMP(int dim, bool planeStrain, Real modulus, Real poissonRatio, Real density, int powerP)
{
	this->InitializeMaterial(dim, planeStrain, modulus, poissonRatio);
	density_ = density;
	powerP_ = powerP;
}

Real getSIMPScaleFactor()
{
	return std::pow(density_, powerP_);
}

Real getSIMPFirstDerivativeScaleFactor()
{
	Real scale;
	if(powerP_ >= 1)
		scale = powerP_ * (std::pow(density_, powerP_-1));
	else
		scale = 0.0;
	
	return scale;
}

Real getSIMPSecondDerivativeScaleFactor()
{
	Real scale;
	if(powerP_ >= 2)
		scale = powerP_ * (powerP_-1) * (std::pow(density_, powerP_-2));
	else
		scale = 0.0;
	
	return scale;
}

void setDensity(Real dens)
{
	density_ = dens;
}

/*
Teuchos::RCP<Intrepid::FieldContainer<Real> > GetMaterialTensor() const
{
	return materialTensor_;
}
*/

// the following function is not in use
Teuchos::RCP<Intrepid::FieldContainer<Real> > computeScaledMaterialTensor(Real scale)
{
	Teuchos::RCP<Intrepid::FieldContainer<Real> > scaledMaterialTensor;
	scaledMaterialTensor = Teuchos::rcp(new Intrepid::FieldContainer<Real>(this->tensorMatSize_, this->tensorMatSize_));
	scaledMaterialTensor -> initialize(0.0);
	
	for(int i=0; i<this->tensorMatSize_; i++)
	{	
 	   for(int j=0; j<this->tensorMatSize_; j++)
	   {
		(*scaledMaterialTensor)(i, j) = scale * (*this->materialTensor_)(i, j);
	   }
	}
	return scaledMaterialTensor;
}

};



#endif

