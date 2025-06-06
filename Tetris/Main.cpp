#include "PyInterface.h"
#include <codecvt>
#include "ASCIIDisplayWindow.h"

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

int main(int argc, char* argv[]) {
    // 检查是否以显示模式启动
	if (argc > 2 && strcmp(argv[1], "--display") == 0) {
		std::wstring pipeName = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.from_bytes(argv[2]);
		runDisplayMode(pipeName.c_str());
		return 0;
	}
	//std::cout << "test text@@" << std::endl;
	//runDisplayMode();
	//Sleep(5000);
	PyInterface pIn;
	pIn.init();
	pIn.run();

    return 0;
}