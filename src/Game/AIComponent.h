#pragma once

class Tank;

class AIComponent {
public:
	AIComponent(Tank* parentTank);

	void update(const double delta);

private:
	Tank* m_parentTank;
};