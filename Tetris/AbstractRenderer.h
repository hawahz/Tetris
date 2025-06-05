#pragma once
#include <string>

namespace abr {
	enum RenderType {
		SUB,
		MAIN
	};
}

class AbstractRenderer {
protected:
	unsigned int width, height;
	
public:
	
	AbstractRenderer(int width, int height);
	void setResolution(unsigned int width, unsigned int height);
	virtual int getWidth() const;
	virtual int getHeight() const;
	void setSubResolution(unsigned int width, unsigned int height);
	void setSubWindow(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
	virtual int getSubWidth() const;
	virtual int getSubHeight() const;
	virtual void clear() = 0;
	virtual void update() = 0;
	virtual void renderBox(int x, int y, abr::RenderType type = abr::RenderType::MAIN) = 0;
	virtual void renderPixel(int x, int y, short color) = 0;
	virtual void renderSubPixel(int x, int y, short color) = 0;
	virtual void renderCharacter(int x, int y, char c) = 0;
	virtual void renderString(int x, int y, std::string message) = 0;
	virtual bool running() = 0;
	short wallColorSide = 255, wallColorBottom = 255, blockColor = 255, shadowColor = 255, wallColorTop = 255;
	unsigned int startX, startY;
	unsigned int subWidth, subHeight;
};

