import nuto
import numpy as np


class Material:
    youngsModulus = 20000.


class Geometry:
    lx = 70.
    ly = 3.1415
    lz = 0.42
    numElements = 10


class BoundaryCondition:
    force = 12.
    displacement = 0.1


def SetupGeometry(structure):
    # create a 1D Truss mesh, add a linear displacement interpolation and update the mesh
    meshInfo = nuto.MeshGenerator.Grid(structure, [Geometry.lx], [Geometry.numElements])
    structure.InterpolationTypeAdd(meshInfo[1], "displacements", "equidistant1")
    structure.ElementTotalConvertToInterpolationType()

    # create and set section
    section = nuto.SectionTruss.Create(Geometry.ly * Geometry.lz)
    structure.ElementTotalSetSection(section)


def SetupMaterial(structure):
    # create and set material law
    materialId = structure.ConstitutiveLawCreate("Linear_Elastic_Engineering_Stress")
    structure.ConstitutiveLawSetParameterDouble(materialId, "Youngs_Modulus", Material.youngsModulus)
    structure.ElementTotalSetConstitutiveLaw(materialId)


def SetupBoundaryConditions(structure, BCType):
    # fix left node at x = [0]
    nodeLeft = structure.NodeGetAtCoordinate(np.array([0.]))
    structure.Constraints().Add(nuto.eDof_DISPLACEMENTS, nuto.Value(nodeLeft))

    # apply nonzero BC at x = length
    nodeRight = structure.NodeGetAtCoordinate(np.array([Geometry.lx]))
    nodeRightId = structure.NodeGetIdAtCoordinate(np.array([Geometry.lx]))
    if BCType == "DisplacmentBC":
        structure.Constraints().Add(nuto.eDof_DISPLACEMENTS, nuto.Value(nodeRight, BoundaryCondition.displacement))
    if BCType == "ForceBC":
        xDirection = np.array([1.0])
        structure.LoadCreateNodeForce(nodeRightId, xDirection, BoundaryCondition.force)


def Solve(structure):
    structure.SolveGlobalSystemStaticElastic()
    intGradient = structure.BuildGlobalInternalGradient()
    extGradient = structure.BuildGlobalExternalLoadVector()
    residual = intGradient.J.Get("Displacements") - extGradient.J.Get("Displacements")
    print("residual: {0}".format(np.linalg.norm(residual)))
    print("(should be _very_ close to zero.)")


def Visualize(structure, file):
    visualizationGroup = structure.GroupGetElementsTotal()

    structure.AddVisualizationComponent(visualizationGroup, "Displacements")
    structure.AddVisualizationComponent(visualizationGroup, "EngineeringStrain")
    structure.AddVisualizationComponent(visualizationGroup, "EngineeringStress")
    structure.ExportVtkDataFileElements(file)


def Run(BCType):
    # create one-dimensional structure
    structure = nuto.Structure(1)

    SetupGeometry(structure)
    SetupMaterial(structure)
    SetupBoundaryConditions(structure, BCType)

    Solve(structure)

    Visualize(structure, "Truss1D2N_" + BCType + ".vtk")


Run("DisplacmentBC")
Run("ForceBC")
