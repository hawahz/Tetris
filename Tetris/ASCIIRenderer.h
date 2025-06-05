#pragma once
#pragma comment(lib, "user32.lib")
#include "AbstractRenderer.h"
#include <string>
#include <iostream>
#include <Windows.h>
#include "ASCIIDisplayWindow.h"

/*
* ASCII Renderer
*/

typedef struct {
	unsigned int x;
	unsigned int y;
} coord;

/*
* 渲染用的所有字符
*/
const std::string charList = " -+o*#&@|O_";
//0    1 -  2 +  3 o  4 *  5 #  6 &  7 @  8 |  9 O  10 _
//" -+>*#&@";
//" .,-'`:!1+*abcdefghijklmnopqrstuvwxyz<>()\\/{}[]?234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ%&@#$"
const int charSize = charList.size();

/*
* void DisableQuickEditMode()
* 令终端编辑无效化
* 防止终端停止刷新
*/
void DisableQuickEditMode();

/*
* void setConsoleWindowSize()
* 设置终端窗口大小
*/
void setConsoleWindowSize(SHORT width, SHORT height);

/*
* class BiStorage
* 存储屏幕像素更新状态的容器
* 使用一系列 unsigned int 类型
* 通过二进制存储更新状态
* 
* unsigned int* biIntChain
* 实际存储单元
* 
* const int chainLen
* 有效存储区域
*/
class BiStorage {
	
public:
	unsigned int* biIntChain;
	const int width;
	const int height;
	const int chainLen;
	BiStorage(int width, int height);

	/*
	* void turnOn(), void turnOff()
	* 将存储单元对应坐标处翻转为正/反
	*/
	void turnOn(int x, int y);
	void turnOff(int x, int y);

	/*
	* 可以用 [{x, y}] 符号来获取 (x, y) 处的像素状态
	*/
	bool operator[](coord pos);

	/*
	* BiOp 是 BiStorage 的子类
	* 通过 BiOp[{x y}] 来获得能通过赋值符号直接操作位于 (x, y) 处的 BiOp 实例
	* 赋值符号 = 能够直接翻转 BiStorage 的单个二进制储存值
	* 
	* BiStorage 通过 BiStorage.bi() 获取中继的 BiOp 实例
	* 中继的 BiOp 实例无法操纵任何数据
	* 只能通过 BiOp[{x, y}] 来获得可以直接操纵单个二进制数据的 BiOp 实例
	*/
	class BiOp {
		const int x, y;
		BiStorage& par;
	public:
		BiOp(int x, int y, BiStorage& par);
		BiOp operator[](coord pos);
		void operator=(bool flag);
	};
	BiOp bi();
};

class ASCIIRenderer : public AbstractRenderer {
	unsigned int scale = 1;
	std::string displayText;
	BiStorage biStorage;
	DisplayWindow window;
	DisplayWindow& out = window;
public:
	ASCIIRenderer(int width, int height);
	void renderBox(int x, int y, abr::RenderType type = abr::RenderType::MAIN);
	void clear() override;
	void update() override;
	void renderPixel(int x, int y, short color) override;
	void renderPixel(int x, int y, char type);
	void renderSubPixel(int x, int y, short color) override;
	void renderCharacter(int x, int y, char c) override;
	void renderString(int x, int y, std::string message) override;
	bool running() override;
};

