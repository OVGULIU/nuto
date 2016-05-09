#pragma once

#include "nuto/mechanics/constitutive/inputoutput/ConstitutiveIOBase.h"
#include <eigen3/Eigen/Core>

namespace NuTo
{


class ConstitutiveMatrixXd : public ConstitutiveIOBase, public Eigen::MatrixXd
{
public:

    ConstitutiveMatrixXd()                                        = default;
    ConstitutiveMatrixXd(const ConstitutiveMatrixXd& )              = default;
    ConstitutiveMatrixXd(      ConstitutiveMatrixXd&&)              = default;

    virtual ~ConstitutiveMatrixXd()                               = default;

    ConstitutiveMatrixXd& operator=(const ConstitutiveMatrixXd& )   = default;
    ConstitutiveMatrixXd& operator=(      ConstitutiveMatrixXd&&)   = default;


    double& operator ()(int rRow, int rCol) override
    {
        return this->Eigen::MatrixXd::operator () (rRow, rCol);
    }


    double operator ()(int rRow, int rCol) const override
    {
        return this->Eigen::MatrixXd::operator () (rRow, rCol);
    }

    void SetZero() override
    {
        this->setZero();
    }

    int GetNumRows() const override
    {
        return rows();
    }

    int GetNumColumns() const override
    {
        return cols();
    }

    virtual Eigen::MatrixXd& AsMatrix()
    {
        return *this;
    }

};

} /* namespace NuTo */
