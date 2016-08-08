#include "AdditiveInputExplicit.h"
#include "nuto/mechanics/constitutive/inputoutput/EngineeringStrain.h"
#include "nuto/mechanics/constitutive/inputoutput/EngineeringStress.h"

void NuTo::AdditiveInputExplicit::AddConstitutiveLaw(NuTo::ConstitutiveBase *rConstitutiveLaw, Constitutive::Input::eInput rModiesInput)
{
    if(mStaticDataAllocated)
        throw MechanicsException(__PRETTY_FUNCTION__,"All constitutive laws have to be attached before static data is allocated!");
    if(rModiesInput == Constitutive::Input::NONE)
    {
        if (mConstitutiveLawOutput != nullptr)
            throw MechanicsException(__PRETTY_FUNCTION__,std::string("There can be only one!!! --- This additive input law only accepts one law which calculates the output. All other laws ")+
                                     " are only allowed to modify the input to this law. Specify the modifying laws by providing the enum of the modified input as second function parameter.");
        mConstitutiveLawOutput = rConstitutiveLaw;
    }
    else
    {
        mModifiedInputs.insert(rModiesInput);
        mConstitutiveLawsModInput.push_back({rConstitutiveLaw,rModiesInput});
    }
    AddCalculableDofCombinations(rConstitutiveLaw);
}


bool NuTo::AdditiveInputExplicit::CheckDofCombinationComputable(NuTo::Node::eDof rDofRow, NuTo::Node::eDof rDofCol, int rTimeDerivative) const
{
    if(mComputableDofCombinations[rTimeDerivative].find(std::pair<Node::eDof,Node::eDof>(rDofRow,rDofCol)) != mComputableDofCombinations[rTimeDerivative].end())
        return true;
    return false;
}

template <int TDim>
NuTo::Error::eError NuTo::AdditiveInputExplicit::EvaluateAdditiveInputExplicit(
        NuTo::ElementBase *rElement, int rIp,
        const NuTo::ConstitutiveInputMap &rConstitutiveInput,
        const NuTo::ConstitutiveOutputMap &rConstitutiveOutput)
{
    using namespace Constitutive;
    static constexpr int VoigtDim = ConstitutiveIOBase::GetVoigtDim(TDim);
    Error::eError error = Error::SUCCESSFUL;

    NuTo::ConstitutiveInputMap copiedInputMap = rConstitutiveInput;
    NuTo::ConstitutiveOutputMap modifiedOutputMap = rConstitutiveOutput;
    modifiedOutputMap[Output::ENGINEERING_STRAIN] = ConstitutiveIOBase::makeConstitutiveIO<TDim>(Output::ENGINEERING_STRAIN);

    auto& engineeringStrain = *static_cast<EngineeringStrain<TDim>*>(copiedInputMap.at(Input::ENGINEERING_STRAIN).get());

    // evaluate sublaws
    for (unsigned int i = 0; i < mConstitutiveLawsModInput.size(); ++i)
    {
        if(modifiedOutputMap.find(Output::D_ENGINEERING_STRESS_D_RELATIVE_HUMIDITY) != modifiedOutputMap.end() &&
                                  mConstitutiveLawsModInput[i].second == Input::ENGINEERING_STRAIN)
        {
            modifiedOutputMap[Output::D_ENGINEERING_STRAIN_D_RELATIVE_HUMIDITY] = ConstitutiveIOBase::makeConstitutiveIO<TDim>(Output::D_ENGINEERING_STRAIN_D_RELATIVE_HUMIDITY);
        }
        if(modifiedOutputMap.find(Output::D_ENGINEERING_STRESS_D_WATER_VOLUME_FRACTION) != modifiedOutputMap.end() &&
                                  mConstitutiveLawsModInput[i].second == Input::ENGINEERING_STRAIN)
        {
            modifiedOutputMap[Output::D_ENGINEERING_STRAIN_D_WATER_VOLUME_FRACTION] = ConstitutiveIOBase::makeConstitutiveIO<TDim>(Output::D_ENGINEERING_STRAIN_D_WATER_VOLUME_FRACTION);
        }
        if(modifiedOutputMap.find(Output::D_ENGINEERING_STRESS_D_TEMPERATURE) != modifiedOutputMap.end() &&
                                  mConstitutiveLawsModInput[i].second == Input::ENGINEERING_STRAIN)
        {
            modifiedOutputMap[Output::D_STRAIN_D_TEMPERATURE] = ConstitutiveIOBase::makeConstitutiveIO<TDim>(Output::D_STRAIN_D_TEMPERATURE);
        }

        // evaluate constitutive law
        switch(TDim)
        {
        case 1:
            error = mConstitutiveLawsModInput[i].first->Evaluate1D(rElement, rIp, rConstitutiveInput, modifiedOutputMap);
            break;
        case 2:
            error = mConstitutiveLawsModInput[i].first->Evaluate2D(rElement, rIp, rConstitutiveInput, modifiedOutputMap);
            break;
        case 3:
            error = mConstitutiveLawsModInput[i].first->Evaluate3D(rElement, rIp, rConstitutiveInput, modifiedOutputMap);
            break;
        default:
            throw MechanicsException(__PRETTY_FUNCTION__,"invalid dimension");
        }
        if(error != Error::SUCCESSFUL)
        {
            throw MechanicsException(__PRETTY_FUNCTION__,"Attached constitutive law returned an error code. Can't handle this");
        }
        for (const auto& it : modifiedOutputMap)
        {
            if (it.first == Output::ENGINEERING_STRAIN)
            {
                const auto& additionalStrain = *static_cast<EngineeringStrain<TDim>*>(modifiedOutputMap.at(Output::ENGINEERING_STRAIN).get());
                engineeringStrain -= additionalStrain;
            }
            else if (rConstitutiveOutput.count(it.first) and rConstitutiveOutput.at(it.first) != nullptr)
            {
                *(rConstitutiveOutput.at(it.first)) = *(it.second);
            }
        }

    }

    // evaluate output law
    switch(TDim)
    {
    case 1:
        error = mConstitutiveLawOutput->Evaluate1D(rElement, rIp, copiedInputMap, rConstitutiveOutput);
        break;
    case 2:
        error = mConstitutiveLawOutput->Evaluate2D(rElement, rIp, copiedInputMap, rConstitutiveOutput);
        break;
    case 3:
        error = mConstitutiveLawOutput->Evaluate3D(rElement, rIp, copiedInputMap, rConstitutiveOutput);
        break;
    default:
        throw MechanicsException(__PRETTY_FUNCTION__,"invalid dimension");
    }

    // evaluate derivatives of outputs depending on sublaws
    for (auto& itOutput : rConstitutiveOutput)
    {
        switch(itOutput.first)
        {
        case Output::D_ENGINEERING_STRESS_D_RELATIVE_HUMIDITY:
        {
            assert(rConstitutiveOutput.count(Output::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN));
            const auto& tangentStressStrain = *static_cast<ConstitutiveMatrix<VoigtDim, VoigtDim>*>(rConstitutiveOutput.at(Output::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN).get());
            const auto& d_EngineeringStrain_D_RH = *static_cast<ConstitutiveVector<VoigtDim>*>(modifiedOutputMap.at(Output::D_ENGINEERING_STRAIN_D_RELATIVE_HUMIDITY).get());
            if(d_EngineeringStrain_D_RH.GetIsCalculated() == false)
                throw MechanicsException(__PRETTY_FUNCTION__,std::string("Necessary value to determine ")+OutputToString(itOutput.first)+" was not calculated!");
            (static_cast<EngineeringStress<TDim>*>(itOutput.second.get()))->AsVector() = tangentStressStrain * d_EngineeringStrain_D_RH;
            break;
        }
        case Output::D_ENGINEERING_STRESS_D_WATER_VOLUME_FRACTION:
        {
            assert(rConstitutiveOutput.count(Output::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN));
            const auto& tangentStressStrain = *static_cast<ConstitutiveMatrix<VoigtDim, VoigtDim>*>(rConstitutiveOutput.at(Output::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN).get());
            const auto& d_EngineeringStrain_D_WV = *static_cast<ConstitutiveVector<VoigtDim>*>(modifiedOutputMap.at(Output::D_ENGINEERING_STRAIN_D_WATER_VOLUME_FRACTION).get());
            if(d_EngineeringStrain_D_WV.GetIsCalculated() == false)
                throw MechanicsException(__PRETTY_FUNCTION__,std::string("Necessary value to determine ")+OutputToString(itOutput.first)+" was not calculated!");
            (static_cast<EngineeringStress<TDim>*>(itOutput.second.get()))->AsVector() = tangentStressStrain * d_EngineeringStrain_D_WV;
            break;
        }
        case Output::D_ENGINEERING_STRESS_D_TEMPERATURE:
        {
            assert(rConstitutiveOutput.count(Output::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN));
            const auto& tangentStressStrain = *static_cast<ConstitutiveMatrix<VoigtDim, VoigtDim>*>(rConstitutiveOutput.at(Output::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN).get());
            const auto& d_Strain_D_Temperature = *static_cast<ConstitutiveVector<VoigtDim>*>(modifiedOutputMap.at(Output::D_STRAIN_D_TEMPERATURE).get());
            if(d_Strain_D_Temperature.GetIsCalculated() == false)
                throw MechanicsException(__PRETTY_FUNCTION__, "Necessary value to determine " + OutputToString(itOutput.first) + " was not calculated!");
            (static_cast<EngineeringStress<TDim>*>(itOutput.second.get()))->AsVector() = tangentStressStrain * d_Strain_D_Temperature;
            break;
        }
        default:
            continue;
        }
        itOutput.second->SetIsCalculated(true);
    }
    return error;
}



NuTo::ConstitutiveInputMap NuTo::AdditiveInputExplicit::GetConstitutiveInputs(
        const NuTo::ConstitutiveOutputMap &rConstitutiveOutput,
        const NuTo::InterpolationType &rInterpolationType) const
{
    ConstitutiveInputMap constitutiveInputMap;

    for (unsigned int i = 0; i < mConstitutiveLawsModInput.size(); ++i)
    {

        ConstitutiveInputMap singleLawInputMap = mConstitutiveLawsModInput[i].first->GetConstitutiveInputs(rConstitutiveOutput,
                                                                                                           rInterpolationType);

        constitutiveInputMap.Merge(singleLawInputMap);
    }

    ConstitutiveInputMap outputLawInputMap = mConstitutiveLawOutput->GetConstitutiveInputs(rConstitutiveOutput,
                                                                                           rInterpolationType);
    constitutiveInputMap.Merge(outputLawInputMap);

    constitutiveInputMap[NuTo::Constitutive::Input::ENGINEERING_STRAIN];

    return constitutiveInputMap;
}



NuTo::Constitutive::Output::eOutput NuTo::AdditiveInputExplicit::GetOutputEnumFromInputEnum(NuTo::Constitutive::Input::eInput rInputEnum)
{
    switch(rInputEnum)
    {
    case Constitutive::Input::ENGINEERING_STRAIN:
        return Constitutive::Output::ENGINEERING_STRAIN;
    default:
        throw MechanicsException(__PRETTY_FUNCTION__,std::string("There is no output enum specified which is related to the input enum ")+Constitutive::InputToString(rInputEnum));
    }
}

void NuTo::AdditiveInputExplicit::AddCalculableDofCombinations(NuTo::ConstitutiveBase *rConstitutiveLaw)
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
