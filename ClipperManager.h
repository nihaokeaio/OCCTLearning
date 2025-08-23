#pragma once
#include <clipper2/clipper.offset.h>
#include <gp_Pnt.hxx>
#include <vector>

class ClipperManager
{
public:
	ClipperManager();
	~ClipperManager();

	void setJointType(Clipper2Lib::JoinType jointType);

	void setEndType(Clipper2Lib::EndType endType);

	void setPath(const std::vector<gp_Pnt>& paths);

	void setOffset(double delta);

	void setTolerence(double tolerence);

	bool perform();

	void getOutPath(std::vector<gp_Pnt>& paths);
private:
	class PImpl;
	std::unique_ptr<PImpl>impl_;
};