// $Id: $

#pragma once

#include "mechanics/timeIntegration/ResultGroupNodeDof.h"

namespace NuTo
{

//! @author Jörg F. Unger, ISM
//! @date October 2009
//! @brief ... standard abstract class for all results
class ResultGroupNodeForce : public ResultGroupNodeDof
{
public:
    //! @brief constructor
    ResultGroupNodeForce(const std::string& rIdent, int rGroupNodeId);

    //! @brief number of dofs (e.g. number of displacement components of a node
    int GetNumData(const StructureBase& rStructure) const override;

    NuTo::eTimeIntegrationResultType GetResultType() const override;

    Eigen::VectorXd CalculateValues(const StructureBase& rStructure,
    		const Eigen::VectorXd& rResidual_j,
    		const Eigen::VectorXd& rResidual_k) const override;

    std::string GetTypeId() const
    {
    	return std::string("ResultGroupNodeForce");
    }

    //! @brief ... Info routine that prints general information about the object (detail according to verbose level)
    void Info() const override
    {

    }

protected:
};
}

//namespace NuTo
