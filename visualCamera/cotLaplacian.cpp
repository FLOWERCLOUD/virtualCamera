#include "cotLaplacian.h"


#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <vector>
#include <Mesh/PolyMesh_ArrayKernelT.hh>

#include <Mesh/PolyMesh_ArrayKernelT.hh>
#include <Mesh/TriMesh_ArrayKernelT.hh>


#include <Scene.h>
using namespace OpenMesh::IO;
//Mesh * mesh_;
//bool open_mesh_file(const std::string & _filename)
//{
//	if (mesh_)
//	{
//		delete mesh_;
//		mesh_ = NULL;
//	}
//	mesh_ = new Mesh;
//	OpenMesh::IO::Options opt;
//	if(!read_mesh(*mesh_, _filename, opt))
//	{
//		delete mesh_;
//		mesh_ = NULL;
//		return false;
//	}
//
//	if(!opt.check(OpenMesh::IO::Options::VertexNormal))
//	{
//		mesh_->update_normals();
//	}
//
//	if(!opt.check(OpenMesh::IO::Options::FaceNormal))
//	{
//		mesh_->update_face_normals();
//	}
//	return true;
//}

void computeEigenCotangentLaplacian(Mesh* _mesh, Eigen::SparseMatrix<double>& cotMat, Eigen::VectorXf& voronoiArea)
{
	using namespace std;
	
	const int nVertices = _mesh->n_vertices();

	// 构建拉普拉斯矩阵
	vector<Eigen::Triplet<double>> tripletList;
	voronoiArea.resize(nVertices);

	int nthVertex = 0;
	struct NearPoint
	{
		int id;
		WVector3 pos;
		float cotPrev, cotNext;
	};
	vector<NearPoint>nearPoints;
	nearPoints.reserve(100);

	for (auto it = _mesh->vertices_begin(); it != _mesh->vertices_end(); ++it, ++nthVertex)
	{
		int nthNeighbour = 0;

		const auto vp = _mesh->point(it.handle());
		const WVector3 centerPoint(vp.values_[0], vp.values_[1], vp.values_[2]);
		nearPoints.clear();

		// 记录邻点坐标
		for (auto itNear = _mesh->vv_begin(it.handle());
			itNear != _mesh->vv_end(it.handle()); ++itNear, ++nthNeighbour)
		{
			int nearVertID = itNear.handle().idx(); //vertexIDMap[itNear.handle()];
			const auto& p = _mesh->point(itNear.handle());
			NearPoint nearPoint;
			nearPoint.id  = nearVertID;
			nearPoint.pos = WVector3(p.values_[0],p.values_[1],p.values_[2]);
			nearPoints.push_back(nearPoint);
		}

		// 计算邻点正切值
		int nNeighbours = nthNeighbour;
		for (nthNeighbour = 0; nthNeighbour < nNeighbours; ++nthNeighbour)
		{
			int nextIdx = (nthNeighbour + 1) % nNeighbours;
			int prevIdx = (nthNeighbour + nNeighbours - 1) % nNeighbours;
			WVector3& curPoint= nearPoints[nthNeighbour].pos;
			WVector3 nextDir = nearPoints[nextIdx].pos - curPoint;
			WVector3 prevDir = nearPoints[prevIdx].pos - curPoint;
			WVector3 centerDir= centerPoint - curPoint;
			nextDir.normalize();
			prevDir.normalize();
			centerDir.normalize();
			double cosNext = centerDir.dot(nextDir);//WVector3::dotProduct(centerDir, nextDir);
			double cosPrev = centerDir.dot(prevDir);
			nearPoints[nthNeighbour].cotPrev = tan(M_PI_2 - acos(cosPrev));
			nearPoints[nthNeighbour].cotNext = tan(M_PI_2 - acos(cosNext));
		}

		// 计算拉普拉斯矩阵值
		double totalCot = 0;
		double totalArea = 0;
		for (nthNeighbour = 0; nthNeighbour < nNeighbours; ++nthNeighbour)
		{
			int nextIdx = (nthNeighbour + 1) % nNeighbours;
			int prevIdx = (nthNeighbour + nNeighbours - 1) % nNeighbours;
			double cotValue = -0.5 * (nearPoints[nextIdx].cotPrev + nearPoints[prevIdx].cotNext);
			int neighID = nearPoints[nthNeighbour].id;
			Eigen::Triplet<double> tri(nthVertex, neighID, cotValue);
			tripletList.push_back(tri);
			totalCot += cotValue;

			// 计算面积
			WVector3& curPoint= nearPoints[nthNeighbour].pos;
			WVector3& nextPoint = nearPoints[nextIdx].pos;
			WVector3  curDir  = curPoint - centerPoint;
			WVector3  nextDir = nextPoint - centerPoint;
			double curLength    = curDir.length();
			double nextLength   = nextDir.length();
			double cosAngle     = curDir.dot( nextDir)/ (curLength * nextLength);
			double sinAngle     = sqrt(1 - cosAngle * cosAngle);
			double area;
			if ( curDir.dot( curDir - nextDir) < 0.0 ||
				nextDir.dot(nextDir - curDir) < 0.0)
				area = curLength * nextLength * sinAngle * 0.25f; // 钝角三角形
			else if(cosAngle < 0.f)					
				area = curLength * nextLength * sinAngle * 0.5f; // 钝角三角形
			else
			{
				double chordLength  = (nextDir - curDir).length();
				double radius       = chordLength / sinAngle * 0.5f;
				double halfCurLength = curLength  * 0.5f;
				double halfNextLength= nextLength * 0.5f;
				double curHeight    = sqrt(radius * radius - halfCurLength * halfCurLength + 1e-5f);
				double nextHeight   = sqrt(radius * radius - halfNextLength * halfNextLength + 1e-5f);
				area = 0.5f * (halfCurLength * curHeight + halfNextLength * nextHeight);
			}
			totalArea += area;
		}
		Eigen::Triplet<double> tri(nthVertex, nthVertex, -totalCot);
		tripletList.push_back(tri);
		voronoiArea[nthVertex] = totalArea;


	}
	cotMat = Eigen::SparseMatrix<double>(nVertices, nVertices);
	cotMat.setFromTriplets(tripletList.begin(),tripletList.end());
}

