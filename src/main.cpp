#include "framework.h"
#include "my_app.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	my_app app(hInstance);
	return app.run(nCmdShow);
}