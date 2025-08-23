#include "ClipperManager.h"
#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <gp_Quaternion.hxx>


class ClipperManager::PImpl
{
public:
	PImpl(ClipperManager* owner) :owner_(owner) {
		jointType_ = Clipper2Lib::JoinType::Round;
		endType_ = Clipper2Lib::EndType::Butt;
		tolerence_ = 1e-6;
	}

	bool checkPathVaild(const std::vector<gp_Pnt>& inputPoints, gp_Pnt& centroid, gp_Dir& normal);

	void convertPath2OCC(const Clipper2Lib::PathD& path, std::vector<gp_Pnt>& outputPoints);
	void convertOCC2Path(const std::vector<gp_Pnt>& inputPoints, Clipper2Lib::PathD& path);

public:
	Clipper2Lib::JoinType jointType_;
	Clipper2Lib::EndType endType_;
	Clipper2Lib::PathD path_;
	double offset_;
	double tolerence_;

	bool isVaild_;
	gp_Trsf transform_;
private:
	ClipperManager* owner_;
};

ClipperManager::ClipperManager()
{
	impl_.reset(new PImpl(this));
}

ClipperManager::~ClipperManager() = default;

void ClipperManager::setJointType(Clipper2Lib::JoinType jointType)
{
	impl_->jointType_ = jointType;
}

void ClipperManager::setEndType(Clipper2Lib::EndType endType)
{
	impl_->endType_ = endType;
}

void ClipperManager::setPath(const std::vector<gp_Pnt>& paths)
{
	impl_->convertOCC2Path(paths, impl_->path_);
	if (impl_->path_.empty())
		return;
}

void ClipperManager::setOffset(double delta)
{
	impl_->offset_ = delta;
}

void ClipperManager::setTolerence(double tolerence)
{
	impl_->tolerence_ = tolerence;
}

bool ClipperManager::perform()
{
	Clipper2Lib::ClipperOffset co;
	Clipper2Lib::Path64 subject;
	for (const auto& p : impl_->path_) {
		subject.emplace_back(Clipper2Lib::Point<int64_t>(static_cast<int>(p.x / impl_->tolerence_), static_cast<int>(p.y / impl_->tolerence_)));
	}


	co.AddPath(subject, impl_->jointType_, impl_->endType_);
	Clipper2Lib::Paths64 solution;
	co.Execute(impl_->offset_ / impl_->tolerence_, solution);

	const auto outer_is_positive = Clipper2Lib::Area(solution) > 0;

	const auto is_positive_func = Clipper2Lib::IsPositive<int64_t>;
	const auto is_positive_count = std::count_if(
		solution.begin(), solution.end(), is_positive_func);

	Clipper2Lib::Path64& ret = solution.front();
	impl_->path_.clear();
	impl_->path_.resize(ret.size());
	for (int i = 0; i < ret.size(); ++i) {
		impl_->path_.at(i) = Clipper2Lib::PointD(static_cast<double>(ret[i].x) * impl_->tolerence_, static_cast<double>(ret[i].y) * impl_->tolerence_);
	}
	return true;
}

void ClipperManager::getOutPath(std::vector<gp_Pnt>& paths)
{
	impl_->convertPath2OCC(impl_->path_, paths);
}

bool ClipperManager::PImpl::checkPathVaild(const std::vector<gp_Pnt>& inputPoints, gp_Pnt& centroid, gp_Dir& normal)
{
	int n = inputPoints.size();
	Eigen::MatrixX3d matPoints(n, 3);
	for (int i = 0; i < n; ++i) {
		matPoints(i, 0) = inputPoints[i].X();
		matPoints(i, 1) = inputPoints[i].Y();
		matPoints(i, 2) = inputPoints[i].Z();
	}
	Eigen::Vector3d meanPoint = matPoints.colwise().mean();
	Eigen::MatrixX3d centered = matPoints.rowwise() - meanPoint.transpose();

	Eigen::Matrix3d cov = (centered.adjoint() * centered) / (matPoints.rows() - 1);

	// 计算特征值
	Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eig(cov);
	Eigen::Vector3d eigenvalues = eig.eigenvalues();
	Eigen::Matrix3d eigenvectors = eig.eigenvectors();

	// 输出特征值和特征向量
	std::cout << "特征值:\n" << eig.eigenvalues() << "\n\n";
	std::cout << "特征向量:\n" << eig.eigenvectors() << std::endl;

	// 最小特征值是否接近0？
	bool isVaild_ = eigenvalues.minCoeff() < 1e-6;
	if (isVaild_) {
		centroid = gp_Pnt(meanPoint.x(), meanPoint.y(), meanPoint.z());
		normal = gp_Dir(eigenvectors.col(0).x(), eigenvectors.col(0).y(), eigenvectors.col(0).z());
	}
	return isVaild_;
}

void ClipperManager::PImpl::convertPath2OCC(const Clipper2Lib::PathD& path, std::vector<gp_Pnt>& outputPoints)
{
	for (const auto& p : path) {
		gp_Pnt point(p.x, p.y, 0);
		point = point.XYZ().Multiplied(transform_.VectorialPart().Transposed());
		point = point.Translated(-gp_Vec(transform_.TranslationPart()));
		outputPoints.emplace_back(point);
	}
}

void ClipperManager::PImpl::convertOCC2Path(const std::vector<gp_Pnt>& inputPoints, Clipper2Lib::PathD& path)
{
	gp_Pnt centroid;
	gp_Dir planeNormal;
	if (checkPathVaild(inputPoints, centroid, planeNormal)) {
		gp_Quaternion quat(gp_Vec(planeNormal), gp_Vec(0, 0, 1));
		gp_Trsf transform;
		transform.SetTranslationPart(-gp_Vec(centroid.Coord()));
		transform.SetRotationPart(quat);
		transform_ = transform;
		for (const auto& p : inputPoints) {
			auto point = (p.Coord() - centroid.Coord()).Multiplied(transform.VectorialPart());
			path.emplace_back(Clipper2Lib::PointD{ point.X(),point.Y() });
		}
	}
}
