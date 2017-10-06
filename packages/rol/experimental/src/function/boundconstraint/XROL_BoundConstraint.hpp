
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

#pragma once

#includes "XROL_VectorTraits.hpp"


namespace XROL {

template<class V>
class BoundConstraint {
public:

  static_assert( implements_elementwise<V>(), 
    "BoundConstraint requires either vector type V to implement elementwise "
    "functions or template specialization of BoundConstraint<V>." );

  using Real = magnitude_t<V>;  

  BoundConstraint( const V& x, 
                   bool isLower = true,
                   Real scale = 1 );

  BoundConstraint( std::unique_ptr<V> lo, 
                   std::unique_ptr<V> up,
                   Real scale =1 );

  virtual ~BoundConstraint();

  virtual void update( const V& );

  
  /** \fn  project
      \brief Project optimization variables onto the bounds.

      This function implements the projection of \f$x\f$ onto the bounds, i.e.,
      \f[
         (P_{[a,b]}(x))(\xi) = \min\{b(\xi),\max\{a(\xi),x(\xi)\}\} \quad \text{for almost every }\xi\in\Xi.
      \f]
       @param[in,out]      x is the optimization variable.
  */
  void project( V& x ) const;
  


  /** \fn projectInterior 
      \brief Project optimization variables into the interior of the feasible set.

      This function implements the projection of \f$x\f$ into the interior of the
      feasible set, i.e.,
      \f[
         (\bar{P}_{[a,b]}(x))(\xi) \in (a(\xi),b(\xi))
           \quad \text{for almost every }\xi\in\Xi.
      \f]
       @param[in,out]      x is the optimization variable.
  */
  void projectInterior( V& x ) const;

  /** \fn pruneUpperActive
      \brief Set variables to zero if they correspond to the upper \f$\epsilon\f$-active set.

      This function sets \f$v(\xi)=0\f$ if \f$\xi\in\mathcal{A}^+_\epsilon(x)\f$.  Here,
      the upper \f$\epsilon\f$-active set is defined as
      \f[
         \mathcal{A}^+_\epsilon(x) = \{\,\xi\in\Xi\,:\,x(\xi) = b(\xi)-\epsilon\,\}.
      \f]
      @param[out]      v   is the variable to be pruned.
      @param[in]       x   is the current optimization variable.
      @param[in]       eps is the active-set tolerance \f$\epsilon\f$. */
  void pruneUpperActive( V& v, 
                         const V& x, 
                         Real eps = 0 ); 

  void pruneUpperActive( V& v, 
                         const dual_t<V>& g, 
                         const V& x, 
                         Real eps = 0 ); 

  void pruneLowerActive( V& v, 
                         const V& x, 
                         Real eps = 0 ); 

  void pruneLowerActive( V& v, 
                         const dual_t<V>& g, 
                         const V& x, 
                         Real eps = 0 ); 

  const V& getUpperBound( void ) const;

  const V& getLowerBound( void ) const;
 
  bool isFeasible( const Vector<Real> &v ) const;

  void activateUpper( void );

  void activateLower( void );

  void activate( void );

  void deactivateLower( void );

  void deactivateUpper( void );

  void deactivate( void );

  bool isUpperActivated( void ) const;

  bool isLowerActivated( void ) const;

  void pruneActive( V& v, const dual_t<V>& g, const V& x, Real eps = 0 ) const;

  void pruneUpperActive( V& v, const dual_t<V>& g, const V& x, Real eps = 0 ) const;

  void pruneLowerActive( V& v, const dual_t<V>& g, const V& x, Real eps = 0 ) const;

  void pruneInactive( V& v, const dual_t<V>& g, const V& x, Real eps = 0 ) const;

  void pruneUpperInactive( V& v, const dual_t<V>& g, const V& x, Real eps = 0 ) const;

  void pruneLowerInactive( V& v, const dual_t<V>& g, const V& x, Real eps = 0 ) const;
  
  void computeProjectedStep( V& v, const V& x ) const;
 
  void computeProjectedGradient( dual_t<V>& g, const V& x ) const;

private:

  std::unique_ptr<V> x_lo_;
  std::unique_ptr<V> x_up_;

  Real scale_;
  Real min_diff_;

  bool Lactivated_;
  bool Uactivated_;

  static constexpr auto INF_ {std::numeric_limits<Real>::max()};
  static constexpr auto NINF_{std::numeric_limits<Real>::lowest()};

};

} // namespace XROL

