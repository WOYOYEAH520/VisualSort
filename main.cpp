#include "MainMenu.h"
#include <Windows.h>

int main() {
	FreeConsole(); // 关闭控制台
	NVisualSort::GetMainMenu(); // 调用 MainMenu 的构造函数
	return 0;
}