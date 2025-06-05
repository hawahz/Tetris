#include "PyInterface.h"

void renderTips(AbstractRenderer& renderer) {
	int height = 12;
	renderer.renderString(renderer.subWidth + 2 + renderer.startX + 3, height++, "A:move left");
	height++;
	renderer.renderString(renderer.subWidth + 2 + renderer.startX + 3, height++, "D:move right");
	height++;
	renderer.renderString(renderer.subWidth + 2 + renderer.startX + 3, height++, "S:fall rapidly");
	height++;
	renderer.renderString(renderer.subWidth + 2 + renderer.startX + 3, height++, "Space:land instantly");
	height++;
	renderer.renderString(renderer.subWidth + 2 + renderer.startX + 3, height++, "P: pause");
}

int main() {
	//std::cout << (int) '\s' << std::endl;
	PyInterface p;
	p.init();
	//std::vector<int> info = p.getInfo();
	p.run();
	Sleep(1000);
	p.init();
	p.run();
}