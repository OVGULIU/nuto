// $Id: SparseMatrixCSRSymmetric.h 494 2011-03-27 19:15:08Z res2002 $

#ifndef SPARSE_MATRIX_CSR_SYMMETRIC_DEF_H
#define SPARSE_MATRIX_CSR_SYMMETRIC_DEF_H

#include "nuto/math/SparseMatrixCSR.h"
#include "nuto/math/FullMatrix.h"
//the template class SparseMatrixCSRVector2Symmetric is split into definition and implementation, include only the definition here
#include "nuto/math/SparseMatrixCSRVector2Symmetric_Def.h"

namespace NuTo
{
template <class T> class SparseMatrixCSRGeneral;
template <class T> class FullMatrix;
//! @author Stefan Eckardt, ISM
//! @date July 2009
//! @brief ... class for symmetric sparse matrices which are stored in CSR format
template <class T>
class SparseMatrixCSRSymmetric : public SparseMatrixCSR<T>
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif  // ENABLE_SERIALIZATION
    friend class NuTo::SparseMatrixCSRVector2Symmetric<T>;
public:
    //! @brief ... constructor
    //! @param rDimension_ ... dimension (number of rows and number of columns) of square matrix
    //! @param rNumReserveEntries_ ... number of entries for which memory is reserved (optional)
    SparseMatrixCSRSymmetric(int rDimension = 0, int rNumReserveEntries= 0);

    //! @brief ... returns whether the matrix is symmetric or unsymmetric
    //! @return true if the matrix is symmetric and false if the matrix is unsymmetric
    bool IsSymmetric() const;

    //! @brief ... returns the number of columns
    //! @return number of columns
    int GetNumColumns() const;

    //! @brief ... add nonzero entry to matrix
    //! @param rRow ... row of the nonzero entry (zero based indexing!!!)
    //! @param rColumn ... column of the nonzero entry (zero based indexing!!!)
    //! @param rValue ... value of the nonzero entry
    void AddEntry(int rRow, int rColumn, T rValue);

    //! @brief ... print info about the object
    void Info() const;

    //! @brief ... import matrix from slang object stored in  a text file
    //! @param rFileName ... file name
    void ImportFromSLangText(const char* rFileName);

    //! @brief ... write nonzero matrix entries into a full matrix
    //! @param rFullMatrix ... the full matrix
    void WriteEntriesToFullMatrix(FullMatrix<T>& rFullMatrix) const;

    //! @brief ... adds \f$(\boldsymbol{A}^T\,\boldsymbol{B}\,\boldsymbol{A})\f$ to the matrix
    //! @param rMatrixA ... matrix A (general sparse matrix in csr storage)
    //! @param rMatrixB ... matrix B (symmetric sparse matrix in csr storage)
    void Add_TransA_Mult_B_Mult_A(const NuTo::SparseMatrixCSRGeneral<T>& rMatrixA, const NuTo::SparseMatrixCSRSymmetric<T>& rMatrixB);

    //! @brief ... subtract t\f$(\boldsymbol{A}^T\boldsymbol{B}^T + \boldsymbol{B} \boldsymbol{A})\f$ from the matrix
    //! @param rMatrixA ... matrix A (general sparse matrix in csr storage)
    //! @param rMatrixB ... matrix B (general sparse matrix in csr storage)
    void Sub_TransA_Mult_TransB_Plus_B_Mult_A(const NuTo::SparseMatrixCSRGeneral<T>& rMatrixA, const NuTo::SparseMatrixCSRGeneral<T>& rMatrixB);

    //! @brief ... multiply sparse matrix with full matrix
    //! @param rMatrix ... full matrix
    //! @return ... result matrix (full storage)
    FullMatrix<T> operator* (const FullMatrix<T>& rMatrix) const;
    
    //! @brief ... multiplies the matrix with an scalar value
    //! @param rOther ... scalar value
    //! @return ... the multiplied matrix (sparse csr storage)
	SparseMatrixCSRSymmetric<T> operator* (const T& rScal) const;

    //! @brief ... Return the name of the class, this is important for the serialize routines, since this is stored in the file
    //!            in case of restoring from a file with the wrong object type, the file id is printed
    //! @return    class name
    std::string GetTypeId()const;

    //! @brief ... resize the matrix and initialize everything to zero
    //! @param  rRow ... number of rows
    //! @param  rCol ... number of columns
    void Resize(int rRow, int rCol);

    //! @brief ... resize the matrix and initialize everything to zero
    //! @param  rRow ... number of rows=number of columns
    void Resize(int rRow);

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::make_nvp("SparseMatrixCSRSymmetric",boost::serialization::base_object< SparseMatrixCSR<T> >(*this));
    }

    //! @brief ... save the object to a file
    //! @param filename ... filename
    //! @param rType ... type of file, either BINARY, XML or TEXT
    void Save ( const std::string &filename, std::string rType)const;

    //! @brief ... restore the object from a file
    //! @param filename ... filename
    //! @param aType ... type of file, either BINARY, XML or TEXT
    void Restore ( const std::string &filename,  std::string rType);
#endif // ENABLE_SERIALIZATION
protected:
};
}
#endif // SPARSE_MATRIX_CSR_SYMMETRIC_DEF_H
