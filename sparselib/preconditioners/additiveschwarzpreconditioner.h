#ifndef ADDITIVESCHWARZPRECONDITIONER_H_
#define ADDITIVESCHWARZPRECONDITIONER_H_

namespace sparse_lib
{

template <typename T, MatrixType Tri, Symmetry Sym, MatrixStorage MatStorage>
class AdditiveSchwarzPreconditionerBase: public Preconditioner<T>
{
public:
	AdditiveSchwarzPreconditionerBase () { assert("Unsupported matrix type." && false); }
	~AdditiveSchwarzPreconditionerBase () { }
	
	void set(const MatrixBase<T>* Mat) { } 
	
	DenseVector<T>& apply(const DenseVector<T>& b, DenseVector<T>& x) { return x; }
	DenseVectorView<T>& apply(const DenseVector<T>& b, DenseVectorView<T>& x) { return x; }
    
	DenseVector<T>& apply(const DenseVectorView<T>& b, DenseVector<T>& x) { return x; }
	DenseVectorView<T>& apply(const DenseVectorView<T>& b, DenseVectorView<T>& x) { return x; }
	
	DenseVector<T>& apply(const DenseVectorTemp<T>& b, DenseVector<T>& x) { return x; }
	DenseVectorView<T>& apply(const DenseVectorTemp<T>& b, DenseVectorView<T>& x) { return x; }
	
	DenseVector<T>& applyTrans(const DenseVector<T>& b, DenseVector<T>& x) { return x; }
	DenseVectorView<T>& applyTrans(const DenseVector<T>& b, DenseVectorView<T>& x) { return x; }
    
	DenseVector<T>& applyTrans(const DenseVectorView<T>& b, DenseVector<T>& x) { return x; }
	DenseVectorView<T>& applyTrans(const DenseVectorView<T>& b, DenseVectorView<T>& x) { return x; }
	
	DenseVector<T>& applyTrans(const DenseVectorTemp<T>& b, DenseVector<T>& x) { return x; }
	DenseVectorView<T>& applyTrans(const DenseVectorTemp<T>& b, DenseVectorView<T>& x) { return x; }
	
private:
	/* data */
};

template <typename T>
class AdditiveSchwarzPreconditionerBase<T, GENERAL, ASYMMETRIC, SCHWARZ_BLOCK>: public Preconditioner<T>
{
public:
	typedef Matrix<MatrixEngine<T, GENERAL, ASYMMETRIC, SCHWARZ_BLOCK> > MatrixType;
	AdditiveSchwarzPreconditionerBase (): M(0) { }
	~AdditiveSchwarzPreconditionerBase () { }
	
	void set(const MatrixBase<T>* Mat);
	void set(const Matrix<MatrixEngine<T, GENERAL, ASYMMETRIC, SCHWARZ_BLOCK> >& Mat);
	
	inline DenseVector<T>& apply(const DenseVector<T>& b, DenseVector<T>& x) { return apply_(b, x); }
	inline DenseVectorView<T>& apply(const DenseVector<T>& b, DenseVectorView<T>& x) { return apply_(b, x); }
	inline DenseVector<T>& apply(const DenseVectorView<T>& b, DenseVector<T>& x) { return apply_(b, x); }
	inline DenseVectorView<T>& apply(const DenseVectorView<T>& b, DenseVectorView<T>& x) { return apply_(b, x); }
	inline DenseVector<T>& apply(const DenseVectorTemp<T>& b, DenseVector<T>& x) { return apply_(b, x); }
	inline DenseVectorView<T>& apply(const DenseVectorTemp<T>& b, DenseVectorView<T>& x) { return apply_(b, x); }
	inline DenseVector<T>& applyTrans(const DenseVector<T>& b, DenseVector<T>& x) { return apply_(b, x); }
	inline DenseVectorView<T>& applyTrans(const DenseVector<T>& b, DenseVectorView<T>& x) { return apply_(b, x); }
	inline DenseVector<T>& applyTrans(const DenseVectorView<T>& b, DenseVector<T>& x) { return apply_(b, x); }
	inline DenseVectorView<T>& applyTrans(const DenseVectorView<T>& b, DenseVectorView<T>& x) { return apply_(b, x); }
	inline DenseVector<T>& applyTrans(const DenseVectorTemp<T>& b, DenseVector<T>& x) { return apply_(b, x); }
	inline DenseVectorView<T>& applyTrans(const DenseVectorTemp<T>& b, DenseVectorView<T>& x) { return apply_(b, x); }
private:
	template <typename V1, typename V2>
	V2& apply_(const V1& b, V2& x);
	
	const MatrixEngine<T, GENERAL, ASYMMETRIC, SCHWARZ_BLOCK> *M;
};

template <typename T>
void AdditiveSchwarzPreconditionerBase<T, GENERAL, ASYMMETRIC, SCHWARZ_BLOCK>::set(const MatrixBase<T>* Mat)
{
	const Matrix<MatrixEngine<T, GENERAL, ASYMMETRIC, SCHWARZ_BLOCK> >* m = dynamic_cast<const Matrix<MatrixEngine<T, GENERAL, ASYMMETRIC, SCHWARZ_BLOCK> >*>(Mat);
	if(m) {
		set(*m);
	} else {
		assert("Wrong Matrix type.  Must be a SchwarzBlock." && false);
	}
}

template <typename T>
void AdditiveSchwarzPreconditionerBase<T, GENERAL, ASYMMETRIC, SCHWARZ_BLOCK>::set(const Matrix<MatrixEngine<T, GENERAL, ASYMMETRIC, SCHWARZ_BLOCK> >& m)
{
	M = &(m.engine());
}

template <typename T> template <typename V1, typename V2>
V2& AdditiveSchwarzPreconditionerBase<T, GENERAL, ASYMMETRIC, SCHWARZ_BLOCK>::apply_(const V1& b, V2& x)
{
	const std::vector<SchwarzLinearSystemBase<T>*> blocks = M->get_blocks();
	const std::vector<int> block_ptr = M->get_block_ptr();
	DenseVector<T> rhs(x.size()), r(x.size());
	//rhs = b - M->get_background()*x;
	//rhs = b;
	M->mv(x, rhs, false, 1.0, 0.0);
	
	r = b - rhs;
	
	int size=blocks.size();
	int i;
	V2 xn(x);
	#pragma omp parallel for default(none) private(i) shared(size, xn, r)
	for(i=0; i<size; ++i) {
		DenseVectorView<T> xv = xn.range(block_ptr[i], block_ptr[i+1]-1);
		DenseVectorView<T> rhsv = r.range(block_ptr[i], block_ptr[i+1]-1);
		if(!blocks[i]->solve(xv,rhsv)){
			abort();
		}
	}
	x+=xn;
	return x;
}

template <typename M>
class AdditiveSchwarzPreconditioner: public AdditiveSchwarzPreconditionerBase<typename M::value_type, M::MT, M::SYM, M::ST> {

};

}


#endif /* end of include guard: ADDITIVESCHWARZPRECONDITIONER_H_ */
