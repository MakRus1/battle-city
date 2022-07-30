#include "AIComponent.h"

#include "GameObjects/Tank.h"

AIComponent::AIComponent(Tank* parentTank) 
	: m_parentTank(parentTank)
{

}

void AIComponent::update(const double delta) {
	m_parentTank->fire();
}