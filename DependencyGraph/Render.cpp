#include "Render.h"

#include <iostream>

void Render::Update(ValueId node)
{
    std::cout << node.m_Id << "update !" << std::endl;
}
