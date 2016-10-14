#include "AdditiveOutput.h"
#include "nuto/base/ErrorEnum.h"
#include "nuto/mechanics/constitutive/ConstitutiveEnum.h"
#include "nuto/mechanics/constitutive/inputoutput/ConstitutiveIOBase.h"
#include "nuto/mechanics/constitutive/inputoutput/ConstitutiveIOMap.h"
#include "nuto/mechanics/constitutive/inputoutput/ConstitutiveVector.h"
#include "nuto/mechanics/constitutive/inputoutput/ConstitutiveScalar.h"
#include "nuto/mechanics/constitutive/inputoutput/EngineeringStress.h"
#include "nuto/mechanics/constitutive/inputoutput/EngineeringStrain.h"
#include "nuto/mechanics/constitutive/staticData/ConstitutiveStaticDataMultipleConstitutiveLaws.h"
#include "nuto/mechanics/nodes/NodeEnum.h"

//VHIRTHAMTODO --- maybe store possible dof combinations one time in a set when constitutive law is added instead of looping over each law every time
void NuTo::AdditiveOutput::AddConstitutiveLaw(NuTo::ConstitutiveBase *rConstitutiveLaw, NuTo::Constitutive::eInput rModiesInput)
{
    if(mStaticDataAllocated)
        throw MechanicsException(__PRETTY_FUNCTION__,"All constitutive laws have to be attached before static data is allocated!");
    mConstitutiveLaws.push_back(rConstitutiveLaw);
    AddCalculableDofCombinations(rConstitutiveLaw);
}

void NuTo::AdditiveOutput::AddConstitutiveLaw(NuTo::ConstitutiveBase *rConstitutiveLaw)
{
    AddConstitutiveLaw(rConstitutiveLaw, NuTo::Constitutive::eInput::NONE);
}

NuTo::ConstitutiveStaticDataBase *NuTo::AdditiveOutput::AllocateStaticData1D(const NuTo::ElementBase *rElement) const
{
    mStaticDataAllocated = true;
    return new ConstitutiveStaticDataMultipleConstitutiveLaws(mConstitutiveLaws,rElement,1);
}

NuTo::ConstitutiveStaticDataBase *NuTo::AdditiveOutput::AllocateStaticData2D(const NuTo::ElementBase *rElement) const
{
    mStaticDataAllocated = true;
    return new ConstitutiveStaticDataMultipleConstitutiveLaws(mConstitutiveLaws,rElement,2);
}

NuTo::ConstitutiveStaticDataBase *NuTo::AdditiveOutput::AllocateStaticData3D(const NuTo::ElementBase *rElement) const
{
    mStaticDataAllocated = true;
    return new ConstitutiveStaticDataMultipleConstitutiveLaws(mConstitutiveLaws,rElement,3);
}

bool NuTo::AdditiveOutput::CheckDofCombinationComputable(NuTo::Node::eDof rDofRow, NuTo::Node::eDof rDofCol, int rTimeDerivative) const
{
    if(mComputableDofCombinations[rTimeDerivative].find(std::pair<Node::eDof,Node::eDof>(rDofRow,rDofCol)) != mComputableDofCombinations[rTimeDerivative].end())
        return true;
    return false;
}

template <int TDim>
NuTo::eError NuTo::AdditiveOutput::Evaluate(NuTo::ElementBase* rElement, int rIp,
        const NuTo::ConstitutiveInputMap &rConstitutiveInput, const NuTo::ConstitutiveOutputMap &rConstitutiveOutput)
{
    using namespace Constitutive;
    eError error = eError::SUCCESSFUL;
    constexpr int VoigtDim = ConstitutiveIOBase::GetVoigtDim(TDim);

    for (auto& output : rConstitutiveOutput)
    {
        if (output.second != nullptr) output.second->SetZero();
    }

    try
    {
        for(unsigned int i = 0; i < mConstitutiveLaws.size(); ++i)
        {
            NuTo::ConstitutiveOutputMap singleOutput;
            for (auto& output : rConstitutiveOutput)
            {
                singleOutput[output.first] = ConstitutiveIOBase::makeConstitutiveIO<TDim>(output.first);
            }

           error = mConstitutiveLaws[i]->Evaluate<TDim>(rElement, rIp, rConstitutiveInput, singleOutput);

            for (const auto& output : singleOutput)
            {
                if (output.second != nullptr and output.second->GetIsCalculated())
                {
                    switch (output.first)
                    {
                    case eOutput::LOCAL_EQ_STRAIN:
                    case eOutput::NONLOCAL_PARAMETER_XI:
                    case eOutput::DAMAGE:
                    case eOutput::EXTRAPOLATION_ERROR:
                    case eOutput::HEAT_CHANGE:
                    case eOutput::D_HEAT_D_TEMPERATURE:
                    case eOutput::INTERNAL_GRADIENT_RELATIVE_HUMIDITY_N:
                    case eOutput::D_INTERNAL_GRADIENT_RH_D_RH_BB_H0:
                    case eOutput::D_INTERNAL_GRADIENT_RH_D_RH_NN_H0:
                    case eOutput::D_INTERNAL_GRADIENT_RH_D_WV_NN_H0:
                    case eOutput::INTERNAL_GRADIENT_WATER_VOLUME_FRACTION_N:
                    case eOutput::D_INTERNAL_GRADIENT_WV_D_WV_BB_H0:
                    case eOutput::D_INTERNAL_GRADIENT_WV_D_WV_NN_H0:
                    case eOutput::D_INTERNAL_GRADIENT_WV_D_RH_NN_H0:
                    case eOutput::D_INTERNAL_GRADIENT_RH_D_RH_NN_H1:
                    case eOutput::D_INTERNAL_GRADIENT_RH_D_WV_NN_H1:
                    case eOutput::D_INTERNAL_GRADIENT_WV_D_WV_NN_H1:
                    case eOutput::INTERNAL_GRADIENT_RELATIVE_HUMIDITY_BOUNDARY_N:
                    case eOutput::INTERNAL_GRADIENT_WATER_VOLUME_FRACTION_BOUNDARY_N:
                    case eOutput::D_INTERNAL_GRADIENT_RH_D_RH_BOUNDARY_NN_H0:
                    case eOutput::D_INTERNAL_GRADIENT_WV_D_WV_BOUNDARY_NN_H0:
                        *static_cast<ConstitutiveScalar*>(rConstitutiveOutput.at(output.first).get()) +=
                            *static_cast<ConstitutiveScalar*>(singleOutput.at(output.first).get());
                        rConstitutiveOutput.at(output.first)->SetIsCalculated(true);
                        break;
                    case eOutput::INTERNAL_GRADIENT_RELATIVE_HUMIDITY_B:
                    case eOutput::INTERNAL_GRADIENT_WATER_VOLUME_FRACTION_B:
                    case eOutput::D_INTERNAL_GRADIENT_RH_D_WV_BN_H0:
                    case eOutput::D_INTERNAL_GRADIENT_WV_D_WV_BN_H0:
                    case eOutput::HEAT_FLUX:
                        *static_cast<ConstitutiveVector<TDim>*>(rConstitutiveOutput.at(output.first).get()) +=
                            *static_cast<ConstitutiveVector<TDim>*>(singleOutput.at(output.first).get());
                        rConstitutiveOutput.at(output.first)->SetIsCalculated(true);
                        break;
                    case eOutput::ENGINEERING_STRESS:
                    case eOutput::D_ENGINEERING_STRESS_D_NONLOCAL_EQ_STRAIN:
                    case eOutput::D_ENGINEERING_STRESS_D_RELATIVE_HUMIDITY:
                    case eOutput::D_ENGINEERING_STRESS_D_WATER_VOLUME_FRACTION:
                    case eOutput::D_LOCAL_EQ_STRAIN_XI_D_STRAIN:
                    case eOutput::D_ENGINEERING_STRESS_D_TEMPERATURE:
                    case eOutput::D_LOCAL_EQ_STRAIN_D_STRAIN:
                        *static_cast<ConstitutiveVector<VoigtDim>*>(rConstitutiveOutput.at(output.first).get()) +=
                            *static_cast<ConstitutiveVector<VoigtDim>*>(singleOutput.at(output.first).get());
                        rConstitutiveOutput.at(output.first)->SetIsCalculated(true);
                        break;
                    case eOutput::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN:
                        *static_cast<ConstitutiveMatrix<VoigtDim, VoigtDim>*>(rConstitutiveOutput.at(output.first).get()) +=
                            *static_cast<ConstitutiveMatrix<VoigtDim, VoigtDim>*>(singleOutput.at(output.first).get());
                        rConstitutiveOutput.at(output.first)->SetIsCalculated(true);
                        break;
                    case eOutput::ENGINEERING_PLASTIC_STRAIN_VISUALIZE:
                    case eOutput::ENGINEERING_STRAIN_VISUALIZE:
                    case eOutput::SHRINKAGE_STRAIN_VISUALIZE:
                    case eOutput::THERMAL_STRAIN:
                        *static_cast<EngineeringStrain<TDim>*>(rConstitutiveOutput.at(output.first).get()) +=
                            *static_cast<EngineeringStrain<TDim>*>(singleOutput.at(output.first).get());
                        rConstitutiveOutput.at(output.first)->SetIsCalculated(true);
                        break;
                    case eOutput::ENGINEERING_STRESS_VISUALIZE:
                        *static_cast<EngineeringStress<TDim>*>(rConstitutiveOutput.at(output.first).get()) +=
                            *static_cast<EngineeringStress<TDim>*>(singleOutput.at(output.first).get());
                        rConstitutiveOutput.at(output.first)->SetIsCalculated(true);
                        break;
                    case eOutput::D_HEAT_FLUX_D_TEMPERATURE_GRADIENT:
                        *static_cast<ConstitutiveMatrix<TDim, TDim>*>(rConstitutiveOutput.at(output.first).get()) +=
                            *static_cast<ConstitutiveMatrix<TDim, TDim>*>(singleOutput.at(output.first).get());
                        rConstitutiveOutput.at(output.first)->SetIsCalculated(true);
                        break;

                    default:
                        throw Exception(__PRETTY_FUNCTION__,
                                        "Output is not implemented or can't be handled.");

                    } // switch outputs
                } // if not nullptr and IsCalculated
            } // for each output
        } // for each sublaw
    } //try
    catch(Exception e)
    {
        e.AddMessage(__PRETTY_FUNCTION__,"Exception while evaluating constitutive law attached to an additive output.");
        throw;
    }
    return error;

}


NuTo::eError NuTo::AdditiveOutput::Evaluate1D(NuTo::ElementBase *rElement, int rIp,
        const NuTo::ConstitutiveInputMap &rConstitutiveInput, const NuTo::ConstitutiveOutputMap &rConstitutiveOutput)
{
    return NuTo::AdditiveOutput::Evaluate<1>(rElement, rIp, rConstitutiveInput, rConstitutiveOutput);
}


NuTo::eError NuTo::AdditiveOutput::Evaluate2D(NuTo::ElementBase *rElement, int rIp,
        const NuTo::ConstitutiveInputMap &rConstitutiveInput, const NuTo::ConstitutiveOutputMap &rConstitutiveOutput)
{
    return NuTo::AdditiveOutput::Evaluate<2>(rElement, rIp, rConstitutiveInput, rConstitutiveOutput);
}


NuTo::eError NuTo::AdditiveOutput::Evaluate3D(NuTo::ElementBase *rElement, int rIp,
        const NuTo::ConstitutiveInputMap &rConstitutiveInput, const NuTo::ConstitutiveOutputMap &rConstitutiveOutput)
{
    return NuTo::AdditiveOutput::Evaluate<3>(rElement, rIp, rConstitutiveInput, rConstitutiveOutput);
}


NuTo::ConstitutiveInputMap NuTo::AdditiveOutput::GetConstitutiveInputs(
        const NuTo::ConstitutiveOutputMap &rConstitutiveOutput, const NuTo::InterpolationType &rInterpolationType) const
{
    ConstitutiveInputMap constitutiveInputMap;

    for(unsigned int i=0; i<mConstitutiveLaws.size(); ++i)
    {
        ConstitutiveInputMap singleLawInputMap = mConstitutiveLaws[i]->GetConstitutiveInputs(rConstitutiveOutput,
                                                                                             rInterpolationType);
        constitutiveInputMap.Merge(singleLawInputMap);
    }

    return constitutiveInputMap;
}

NuTo::Constitutive::eConstitutiveType NuTo::AdditiveOutput::GetType() const
{
    return NuTo::Constitutive::eConstitutiveType::ADDITIVE_OUTPUT;
}


void NuTo::AdditiveOutput::AddCalculableDofCombinations(NuTo::ConstitutiveBase *rConstitutiveLaw)
{
    std::set<Node::eDof> allDofs = Node::GetDofSet();
    for (unsigned int i=0; i<mComputableDofCombinations.size(); ++i)
        for (auto itRow : allDofs)
            for (auto itCol : allDofs)
            {
                if (rConstitutiveLaw->CheckDofCombinationComputable(itRow,itCol,i))
                        mComputableDofCombinations[i].emplace(itRow,itCol);
            }
}
