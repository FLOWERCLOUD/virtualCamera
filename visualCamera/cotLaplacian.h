#pragma once
#define _USE_MATH_DEFINES

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <IO/MeshIO.hh>
#include <Mesh/PolyMesh_ArrayKernelT.hh>
#include <Mesh/TriMesh_ArrayKernelT.hh>
 /** this structure defines the traits of the mesh 
        */
using namespace OpenMesh::IO;
struct CustomTraits : OpenMesh::DefaultTraits 
{
	// let Point and Normal be a vector made from doubles
	typedef OpenMesh::Vec3d Point;
	typedef OpenMesh::Vec3d Normal;

	// add normal property to vertices and faces
	VertexAttributes  (OpenMesh::Attributes::Normal | OpenMesh::Attributes::Color | OpenMesh::Attributes::Status);
	FaceAttributes    (OpenMesh::Attributes::Normal);
	HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge); 
};
typedef OpenMesh::TriMesh_ArrayKernelT<CustomTraits>  TriangularMesh;
typedef OpenMesh::PolyMesh_ArrayKernelT<CustomTraits> Mesh;

////namespace SpectralProcess
////{
void computeEigenCotangentLaplacian(Mesh* _mesh, Eigen::SparseMatrix<double>& cotMat, Eigen::VectorXf& voronoiArea);
////}
