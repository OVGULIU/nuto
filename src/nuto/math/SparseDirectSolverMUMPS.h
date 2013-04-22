// $Id$
#ifndef SPARSE_DIRECT_SOLVER_MUMPS_H
#define SPARSE_DIRECT_SOLVER_MUMPS_H

#include "nuto/math/FullMatrix_Def.h"
#include "nuto/math/SparseDirectSolver.h"

namespace NuTo
{
// forward declarations
template<class T> class SparseMatrixCSR;
template<class T, int rows> class FullVector;

//! @author Stefan Eckardt, ISM
//! @date October 2009
//! @brief ... interface for the MUMPS sparse direct solver
class SparseDirectSolverMUMPS : public SparseDirectSolver
{
public:
    //! @brief ... default constructor
    SparseDirectSolverMUMPS();

    //! @brief ... print information about the class attributes
    virtual void Info()const
    {
    }

    std::string GetTypeId()const
    {
        return std::string("SparseDirectSolverMUMPS");
    }

    //! @brief ... solve system of equations: rMatrix * rSolution = rRhs
    //! @param rMatrix ... sparse coefficient matrix, stored in compressed CSR format (input)
    //! @param rRhs ... matrix storing the right-hand-side vectors (input)
    //! @param rSolution ... matrix storing the corresponding solution vectors (output)
    void Solve(const NuTo::SparseMatrixCSR<double>& rMatrix, const NuTo::FullVector<double,Eigen::Dynamic>& rRhs, NuTo::FullVector<double,Eigen::Dynamic>& rSolution);

    //! @brief ... calculates the Schurcomplement
    //! @param rMatrix ... sparse coefficient matrix, stored in compressed CSR format (input)
    //! @param rSchurIndices ... vector storing the indices of the global matrix to be condensed to (zero based indexing)
    //! @param rSchurComplement ... Schur complement
    void SchurComplement(const NuTo::SparseMatrixCSR<double>& rMatrix, NuTo::FullVector<int, Eigen::Dynamic> rSchurIndices, NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rSchurComplement);
#ifdef HAVE_MUMPS

protected:
    //! @brief ... generate an error message from the error code
    //! @param error ... error code
    //! @return error message as std::string
    std::string GetErrorString(int error) const;
#endif // HAVE_MUMPS

};
}
#endif // SPARSE_DIRECT_SOLVER_MUMPS_H
