
#pragma once
namespace clan
{

class ModelDataAttachmentPoint
{
public:
	ModelDataAttachmentPoint() : bone_selector(-1) { }
	Vec3f position;
	int bone_selector;
};

}
