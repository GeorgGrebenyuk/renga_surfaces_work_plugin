#include "renga_surfaces.h"
#include <Renga/CreateApplication.hpp>


class TextHandler : public Renga::ActionEventHandler
{
public:
    TextHandler(Renga::IActionPtr pAction, const std::wstring& text) :
        Renga::ActionEventHandler(pAction),
        m_text(text)
    {
    }

    void OnTriggered() override
    {
        ::MessageBox(nullptr, (LPCWSTR)m_text.c_str(), (LPCWSTR)L"Plugin message", MB_ICONINFORMATION | MB_OK);
    }

    void OnToggled(bool checked) override {}

private:
    std::wstring m_text;
};

renga_surfaces::renga_surfaces()
{
    ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
}

renga_surfaces::~renga_surfaces()
{
    ::CoUninitialize();
}
void renga_surfaces::addHandler(Renga::ActionEventHandler* pHandler)
{
    m_handlerContainer.emplace_back(HandlerPtr(pHandler));
}

bool renga_surfaces::initialize(const wchar_t* pluginDir)
{
    auto pApplication = Renga::CreateApplication();
    if (!pApplication)
        return false;

    if (auto pUI = pApplication->GetUI())
    {
        if (auto pUIPanelExtension = pUI->CreateUIPanelExtension())
        {
            
            auto pAction = pUI->CreateAction();
            pAction->PutDisplayName("SimpleButton");

            addHandler(new TextHandler(pAction, L"SimpleButton2"));

            pUIPanelExtension->AddToolButton(
                pAction);

            pUI->AddExtensionToPrimaryPanel(pUIPanelExtension);
            pUI->AddExtensionToActionsPanel(pUIPanelExtension, Renga::ViewType::ViewType_View3D);
        }
    }
    return true;
}

void renga_surfaces::stop()
{
    m_handlerContainer.clear();
}