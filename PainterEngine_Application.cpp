#include <vector>
#include <string>
#include <exception>
#include "PainterEngine_Application.h"
using namespace std;
using ints = vector<int>;
bool solve(ints& state);
PX_Application App;

PX_FontModule fm;
PX_Object* root;
PX_Object* pButton;
ints allnums{ 1,2,3,4,5,6,7,8,9 };
template<int width,int  height,int x_num, int y_num>
class Editeer {
public:
	void create(px_memorypool* mp) {
		for (int _y = 0; _y < y_num; _y ++)
			for (int _x = 0; _x < x_num; _x ++)
				edits.push_back(PX_Object_EditCreate(mp, nullptr, _x*width, _y*height, width, height, nullptr));
	}
	void SetMaxTextLength(int length) {
		for (const auto& _edit : edits)PX_Object_EditSetMaxTextLength(_edit, length);
	}
	void Render(px_surface* ps, unsigned elapsed) {
		for (const auto& _edit : edits) {
			try {
			PX_ObjectRender(ps, _edit, elapsed);
			}
			catch (exception& e) {
				return;
			}
		}
	}
	void PostEvent(PX_Object_Event e) {
		for (const auto& _edit : edits)PX_ObjectPostEvent(_edit, e);
	}
	void SetLimit(const char* limit) {
		for (const auto& _edit : edits)PX_Object_EditSetLimit(_edit, limit);
	}
	ints GetAllInts() {
		ints res;
		for (const auto& _edit : edits)
			if (char* x = PX_Object_EditGetText(_edit); x!="")res.push_back(atoi(x));
			else res.push_back(0);
		return res;
	}
	void PutIn(ints& ok) {

		for (auto at{ edits.begin() }; at != edits.end(); at++)
			if (string text(PX_Object_EditGetText(*at));text.empty()) {
				char buff[5]{ 0 };
				_itoa_s(ok.at(at - edits.begin()), buff, 10);
				PX_Object_EditSetText(*at, buff);
			}
	}
private:
	vector<PX_Object*> edits;
};
using Board = Editeer<40, 40, 9, 9>;
Board Editer;
px_void APP_EditItemRender(px_surface* psurface, PX_Object* pObjectItem, px_dword elpased) {
	char* Content = PX_Object_EditGetText(pObjectItem);
	PX_FontModuleDrawText(psurface, NULL, pObjectItem->x + 2, pObjectItem->y + 2, PX_ALIGN_LEFTTOP, Content, PX_COLOR(255, 255, 255, 255));
}
px_bool APP_EditItemOnCreate(px_memorypool* mp, PX_Object* ItemObject, px_void* usesptr) {
	ItemObject->Func_ObjectRender = APP_EditItemRender;
	return PX_TRUE;
}
void ButtonCallBack(PX_Object*, PX_Object_Event e, px_void* user_ptr) {
	ints board{ ((Board*)user_ptr)->GetAllInts() };
	solve(board);
	((Board*)user_ptr)->PutIn(board);
}
px_bool PX_ApplicationInitialize(PX_Application* pApp, px_int screen_width, px_int screen_height)
{
	PX_ApplicationInitializeDefault(&pApp->runtime, screen_width, screen_height);//默认显示函数
	root = PX_ObjectCreate(&pApp->runtime.mp_ui, PX_NULL, 0, 0, 0, 0, 0, 0);//创建根对象
	/*pEdit = PX_Object_EditCreate(&pApp->runtime.mp_ui, PX_NULL, 0, 0, 50, 50, NULL);*/
	Editer.create(&pApp->runtime.mp_ui);
	Editer.SetMaxTextLength(1);
	Editer.SetLimit("123456789");
	pButton = PX_Object_PushButtonCreate(&pApp->runtime.mp_ui, PX_NULL, 450, 450, 70, 50, "Get it!", NULL);
	PX_ObjectRegisterEvent(pButton, PX_OBJECT_EVENT_EXECUTE, ButtonCallBack, &Editer);
	return PX_TRUE;
}

px_void PX_ApplicationUpdate(PX_Application* pApp, px_dword elpased)
{
	PX_ObjectUpdate(root, elpased);
}

px_void PX_ApplicationRender(PX_Application* pApp, px_dword elpased)
{
	px_surface* pRenderSurface = &pApp->runtime.RenderSurface;
	PX_RuntimeRenderClear(&pApp->runtime, PX_OBJECT_UI_DEFAULT_BACKGROUNDCOLOR);
	PX_ObjectRender(pRenderSurface, root, elpased);
	PX_ObjectRender(pRenderSurface, pButton, elpased);
	Editer.Render(pRenderSurface, elpased);
}

px_void PX_ApplicationPostEvent(PX_Application* pApp, PX_Object_Event e)
{
	PX_ApplicationEventDefault(&pApp->runtime, e);
	PX_ObjectPostEvent(root, e);
	PX_ObjectPostEvent(pButton, e);
	Editer.PostEvent(e);
}
inline bool in(auto&& container, auto&& item) {
	return find(container.begin(), container.end(), item) != container.end();
}
ints what_can_put(ints& state, ints::iterator pos) {
	auto x = (pos - state.begin()) % 9, y = (pos - state.begin()) / 9;
	ints had(pos - x, pos - x + 9);
	for (int i = 0; i < 9; ++i)had.push_back(state.at(i * 9 + x));
	for (int i = x / 3 * 3; i < x / 3 * 3 + 3; ++i)for (int j = y / 3 * 3; j < y / 3 * 3 + 3; ++j)
		had.push_back(state.at(j * 9 + i));
	ints res;
	for (auto& i : allnums) if (!in(had, i))res.push_back(i);
	return res;
}
bool solve(ints& state) {
	if (!in(state, 0))return true;
	for (auto at{ state.begin() }; at != state.end(); ++at) if (*at == 0) {
		auto cans{ what_can_put(state, at) };
		for (auto&& v : cans) {
			*at = v;
			if (solve(state))return true;
		}
		*at = 0; return false;
	}
}