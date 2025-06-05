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
* ��Ⱦ�õ������ַ�
*/
const std::string charList = " -+o*#&@|O_";
//0    1 -  2 +  3 o  4 *  5 #  6 &  7 @  8 |  9 O  10 _
//" -+>*#&@";
//" .,-'`:!1+*abcdefghijklmnopqrstuvwxyz<>()\\/{}[]?234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ%&@#$"
const int charSize = charList.size();

/*
* void DisableQuickEditMode()
* ���ն˱༭��Ч��
* ��ֹ�ն�ֹͣˢ��
*/
void DisableQuickEditMode();

/*
* void setConsoleWindowSize()
* �����ն˴��ڴ�С
*/
void setConsoleWindowSize(SHORT width, SHORT height);

/*
* class BiStorage
* �洢��Ļ���ظ���״̬������
* ʹ��һϵ�� unsigned int ����
* ͨ�������ƴ洢����״̬
* 
* unsigned int* biIntChain
* ʵ�ʴ洢��Ԫ
* 
* const int chainLen
* ��Ч�洢����
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
	* ���洢��Ԫ��Ӧ���괦��תΪ��/��
	*/
	void turnOn(int x, int y);
	void turnOff(int x, int y);

	/*
	* ������ [{x, y}] ��������ȡ (x, y) ��������״̬
	*/
	bool operator[](coord pos);

	/*
	* BiOp �� BiStorage ������
	* ͨ�� BiOp[{x y}] �������ͨ����ֵ����ֱ�Ӳ���λ�� (x, y) ���� BiOp ʵ��
	* ��ֵ���� = �ܹ�ֱ�ӷ�ת BiStorage �ĵ��������ƴ���ֵ
	* 
	* BiStorage ͨ�� BiStorage.bi() ��ȡ�м̵� BiOp ʵ��
	* �м̵� BiOp ʵ���޷������κ�����
	* ֻ��ͨ�� BiOp[{x, y}] ����ÿ���ֱ�Ӳ��ݵ������������ݵ� BiOp ʵ��
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

