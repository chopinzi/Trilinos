#include "EpetraModelEval2DSim.hpp"
#include "Teuchos_ScalarTraits.hpp"
#include "Epetra_SerialComm.h"
#include "Epetra_CrsMatrix.h"

EpetraModelEval2DSim::EpetraModelEval2DSim(
  const double         d
  ,const double        p0
  ,const double        p1
  ,const double        x00
  ,const double        x01
  ,const bool          showGetInvalidArg
  )
  :d_(d),showGetInvalidArg_(showGetInvalidArg)
{
  using Teuchos::rcp;

#ifdef HAVE_MPI
	epetra_comm_ = rcp(new Epetra_MpiComm(MPI_COMM_WORLD));
#else
	epetra_comm_ = rcp(new Epetra_SerialComm());
#endif

  const int nx = 2;

  map_x_ = rcp(new Epetra_Map(nx,0,*epetra_comm_));
  x0_ = rcp(new Epetra_Vector(*map_x_));  (*x0_)[0] = x00; (*x0_)[1] = x01;
  p_  = rcp(new Epetra_Vector(*map_x_));  (*p_)[0] = p0; (*p_)[1] = p1;

  // Initialize the graph for W CrsMatrix object
  W_graph_ = rcp(new Epetra_CrsGraph(::Copy,*map_x_,nx));
  {
    int indices[nx] = { 0, 1 };
    for( int i = 0; i < nx; ++i )
      W_graph_->InsertGlobalIndices(i,nx,indices);
  }
  W_graph_->FillComplete();

  isInitialized_ = true;

}

// Overridden from EpetraExt::ModelEvaluator

Teuchos::RefCountPtr<const Epetra_Map>
EpetraModelEval2DSim::get_x_map() const
{
  return map_x_;
}

Teuchos::RefCountPtr<const Epetra_Map>
EpetraModelEval2DSim::get_f_map() const
{
  return map_x_;
}

Teuchos::RefCountPtr<const Epetra_Vector>
EpetraModelEval2DSim::get_x_init() const
{
  return x0_;
}

Teuchos::RefCountPtr<Epetra_Operator>
EpetraModelEval2DSim::create_W() const
{
  return Teuchos::rcp(new Epetra_CrsMatrix(::Copy,*W_graph_));
}

EpetraExt::ModelEvaluator::InArgs
EpetraModelEval2DSim::createInArgs() const
{
  InArgsSetup inArgs;
  inArgs.setModelEvalDescription(this->description());
  inArgs.setSupports(IN_ARG_x,true);
  inArgs.setSupports(IN_ARG_beta,true);
  return inArgs;
}

EpetraExt::ModelEvaluator::OutArgs
EpetraModelEval2DSim::createOutArgs() const
{
  OutArgsSetup outArgs;
  outArgs.setModelEvalDescription(this->description());
  outArgs.setSupports(OUT_ARG_f,true);
  outArgs.setSupports(OUT_ARG_W,true);
  outArgs.set_W_properties(
    DerivativeProperties(
      DERIV_LINEARITY_NONCONST
      ,DERIV_RANK_FULL
      ,true // supportsAdjoint
      )
    );
  return outArgs;
}

void EpetraModelEval2DSim::evalModel( const InArgs& inArgs, const OutArgs& outArgs ) const
{
  using Teuchos::dyn_cast;
  using Teuchos::rcp_dynamic_cast;
  //
  // Get the input arguments
  //
  const Epetra_Vector &x = *inArgs.get_x();
  //
  // Get the output arguments
  //
  Teuchos::RefCountPtr<Epetra_Vector>       f_inout = outArgs.get_f();
  Teuchos::RefCountPtr<Epetra_Operator>     W_inout = outArgs.get_W();
  if(showGetInvalidArg_) {
    Teuchos::RefCountPtr<Epetra_Vector> g1_inout = outArgs.get_g(1);
  }
  //
  // Compute the functions
  //
  const Epetra_Vector &p = *p_;
  if(f_inout.get()) {
    Epetra_Vector &f = *f_inout;
    // zero-based indexing for Epetra!
    f[0] =        x[0]      + x[1]*x[1] - p[0];
    f[1] = d_ * ( x[0]*x[0] - x[1]      - p[1] );
  }
  if(W_inout.get()) {
    const double beta = inArgs.get_beta();
    Epetra_CrsMatrix &DfDx = dyn_cast<Epetra_CrsMatrix>(*W_inout);
    DfDx.PutScalar(0.0);
    //
    // Fill W = beta*DfDx
    //
    // W = beta*DfDx = beta * [      1.0,  2*x[1] ]
    //                        [ 2*d*x[0],     -d  ]
    //
    double values[2];
    int indexes[2];
    // Row [0]
    values[0] = beta;               indexes[0] = 0;
    values[1] = 2.0*beta*x[1];      indexes[1] = 1;
    DfDx.SumIntoGlobalValues( 0, 2, values, indexes );
    // Row [1]
    values[0] = 2.0*beta*d_*x[0];   indexes[0] = 0;
    values[1] = -beta*d_;           indexes[1] = 1;
    DfDx.SumIntoGlobalValues( 1, 2, values, indexes );
  }
}
