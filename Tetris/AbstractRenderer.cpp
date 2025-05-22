#include "AbstractRenderer.h"

AbstractRenderer::AbstractRenderer(int width, int height) : width(width), height(height), startX(0), startY(0), subWidth(0), subHeight(0) {
}

void AbstractRenderer::setResolution(unsigned int width, unsigned int height) {
	this->width = width;
	this->height = height;
}

int AbstractRenderer::getWidth() const {
	return this->width;
}

int AbstractRenderer::getHeight() const {
	return this->height;
}

void AbstractRenderer::setSubResolution(unsigned int width, unsigned int height) {
	this->subWidth = width;
	this->subHeight = height;
}

void AbstractRenderer::setSubWindow(unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
	this->startX = x;
	this->startY = y;
	this->setSubResolution(width, height);
}

int AbstractRenderer::getSubWidth() const {
	return this->subWidth;
}

int AbstractRenderer::getSubHeight() const {
	return this->subHeight;
}




