// @HEADER
// ***********************************************************************
// 
//    Thyra: Interfaces and Support for Abstract Numerical Algorithms
//                 Copyright (2004) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ***********************************************************************
// @HEADER


#ifndef THYRA_DEFAULT_SERIAL_DENSE_LINEAR_OP_WITH_SOLVE_HPP
#define THYRA_DEFAULT_SERIAL_DENSE_LINEAR_OP_WITH_SOLVE_HPP


#include "Thyra_DefaultSerialDenseLinearOpWithSolveDecl.hpp"
#include "Thyra_LinearOpWithSolveBase.hpp"
#include "Thyra_SingleScalarLinearOpWithSolveBase.hpp"
#include "Thyra_DetachedMultiVectorView.hpp"
#include "Thyra_AssertOp.hpp"
#include "Teuchos_Assert.hpp"


namespace Thyra {

  
// Constructors/initializers/accessors


template<class Scalar>
DefaultSerialDenseLinearOpWithSolve<Scalar>::DefaultSerialDenseLinearOpWithSolve()
{}


template<class Scalar>
void DefaultSerialDenseLinearOpWithSolve<Scalar>::initialize(
  const RCP<const MultiVectorBase<Scalar> > &M )
{
  using Teuchos::outArg;
#ifdef TEUCHOS_DEBUG
  TEUCHOS_ASSERT(!is_null(M));
  TEUCHOS_ASSERT(isFullyInitialized(*M));
  TEUCHOS_ASSERT(M->range()->hasInCoreView());
  TEUCHOS_ASSERT(M->domain()->hasInCoreView());
  THYRA_ASSERT_VEC_SPACES("", *M->range(), *M->domain());
#endif
  factorize(*M, outArg(LU_), outArg(ipiv_));
  M_ = M;
}


// Overridden from LinearOpBase


template<class Scalar>
RCP<const VectorSpaceBase<Scalar> >
DefaultSerialDenseLinearOpWithSolve<Scalar>::range() const
{
  if (!is_null(M_))
    return M_->range();
  return Teuchos::null;
}


template<class Scalar>
RCP<const VectorSpaceBase<Scalar> >
DefaultSerialDenseLinearOpWithSolve<Scalar>::domain() const
{
  if (!is_null(M_))
    return M_->domain();
  return Teuchos::null;
}


// protected


// Overridden from SingleScalarLinearOpBase


template<class Scalar>
bool DefaultSerialDenseLinearOpWithSolve<Scalar>::opSupported(
  ETransp M_trans) const
{
  return Thyra::opSupported(*M_, M_trans);
}


template<class Scalar>
void DefaultSerialDenseLinearOpWithSolve<Scalar>::apply(
  const ETransp M_trans,
  const MultiVectorBase<Scalar> &X,
  MultiVectorBase<Scalar> *Y,
  const Scalar alpha,
  const Scalar beta
  ) const
{
  Thyra::apply( *M_, M_trans, X, Y, alpha, beta );
}


// Overridden from SingleScalarLinearOpWithSolveBase


template<class Scalar>
bool DefaultSerialDenseLinearOpWithSolve<Scalar>::solveSupportsTrans(
  ETransp M_trans) const
{
  typedef Teuchos::ScalarTraits<Scalar> ST;
  return ( ST::isComplex ? ( M_trans!=CONJ ) : true );
}


template<class Scalar>
bool DefaultSerialDenseLinearOpWithSolve<Scalar>::solveSupportsSolveMeasureType(
  ETransp M_trans, const SolveMeasureType& solveMeasureType) const
{
  // We support all solve measures since we are a direct solver
  return this->solveSupportsTrans(M_trans);
}


template<class Scalar>
void DefaultSerialDenseLinearOpWithSolve<Scalar>::solve(
  const ETransp M_trans,
  const MultiVectorBase<Scalar> &B,
  MultiVectorBase<Scalar> *X,
  const int numBlocks,
  const BlockSolveCriteria<Scalar> blockSolveCriteria[],
  SolveStatus<Scalar> blockSolveStatus[]
  ) const
{
#ifdef TEUCHOS_DEBUG
  TEUCHOS_ASSERT(X!=0);
  THYRA_ASSERT_LINEAR_OP_MULTIVEC_APPLY_SPACES(
    "DefaultSerialDenseLinearOpWithSolve<Scalar>::solve(...)",
    *this, M_trans, *X, &B );
#endif
  backsolve( LU_, ipiv_, M_trans, B, Teuchos::ptr(X) );
  for (int i = 0; i < numBlocks; ++i ) {
    blockSolveStatus[i] = SolveStatus<Scalar>();
    blockSolveStatus[i].solveStatus = SOLVE_STATUS_CONVERGED;
  }
}


// private


template<class Scalar>
void DefaultSerialDenseLinearOpWithSolve<Scalar>::factorize(
  const MultiVectorBase<Scalar> &M,
  const Ptr<RTOpPack::ConstSubMultiVectorView<Scalar> > &LU,
  const Ptr<Array<Teuchos_Index> > &ipiv
  )
{
  using Teuchos::outArg;
  const ConstDetachedMultiVectorView<Scalar> dM(M);
  const int dim = dM.subDim();
  ipiv->resize(dim);
  RTOpPack::SubMultiVectorView<Scalar> LU_tmp(dim, dim);
  RTOpPack::assign_entries<Scalar>( outArg(LU_tmp), dM.smv() );
  Teuchos_Index rank = -1;
  RTOpPack::getrf<Scalar>( LU_tmp, (*ipiv)(), outArg(rank) );
  TEUCHOS_ASSERT_EQUALITY( dim, rank );
  *LU = LU_tmp; // Weak copy
}


template<class Scalar>
void DefaultSerialDenseLinearOpWithSolve<Scalar>::backsolve(
  const RTOpPack::ConstSubMultiVectorView<Scalar> &LU,
  const ArrayView<const int> ipiv,
  const ETransp transp,
  const MultiVectorBase<Scalar> &B,
  const Ptr<MultiVectorBase<Scalar> > &X
  )
{
  using Teuchos::outArg;
  assign( X, B );
  DetachedMultiVectorView<Scalar> dX(*X);
  RTOpPack::getrs<Scalar>( LU, ipiv, convertToRTOpPackETransp(transp),
    outArg(dX.smv()) );
}


}	// end namespace Thyra


#endif	// THYRA_DEFAULT_SERIAL_DENSE_LINEAR_OP_WITH_SOLVE_HPP
