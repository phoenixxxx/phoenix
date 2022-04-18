#include <Window/Window.h>
#include <D3D11/D3D11Manager.h>
#include <D3D11LineRenderer/D3D11LineRenderer.h>
#include <D3D11MeshRenderer/D3D11MeshRenderer.h>
#include <Utils/D3D11imgui.h>
#include <Utils/Console.h>

#include <Scenario/Visualize.h>
#include <Utils/Event.h>

//#define CONSOLE

#ifdef CONSOLE
void PipeToSTDOut()
{
    FILE* fpstdin = stdin, * fpstdout = stdout, * fpstderr = stderr;
    freopen_s(&fpstdin, "CONIN$", "r", stdin);
    freopen_s(&fpstdout, "CONOUT$", "w", stdout);
    freopen_s(&fpstderr, "CONOUT$", "w", stderr);

    HANDLE hStdout = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    SetStdHandle(STD_OUTPUT_HANDLE, hStdout);
    SetStdHandle(STD_ERROR_HANDLE, hStdout);
}
#endif

Phoenix::Scenario* currentScenario = nullptr;
void Resize(uint32_t width, uint32_t height)
{
    Phoenix::D3D11Manager::Instance()->ResizeSwapChain(Phoenix::Window::Instance()->GetCurrenSize(), Phoenix::Window::Instance()->GetWindowHandle());
    Phoenix::Console::Instance()->Resize(Phoenix::Window::Instance()->GetCurrenSize());
    if (currentScenario)
        currentScenario->Resize(width, height);
}

void MouseClick(const Phoenix::MouseInfo& info)
{
    if (currentScenario)
    {
        if (!ImGui::GetIO().WantCaptureMouse)
            currentScenario->MouseClick(info);
    }
}

void MouseDrag(const Phoenix::MouseInfo& info)
{
    if (currentScenario)
    {
        if (!ImGui::GetIO().WantCaptureMouse)
            currentScenario->MouseDrag(info);
    }
}

void MouseMove(const Phoenix::MouseInfo& info)
{
    if (currentScenario)
    {
        if (!ImGui::GetIO().WantCaptureMouse)
            currentScenario->MouseMove(info);
    }
}

void MouseWheel(const Phoenix::MouseInfo& info)
{
    if (currentScenario)
    {
        if (!ImGui::GetIO().WantCaptureMouse)
            currentScenario->MouseWheel(info);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
#ifdef CONSOLE
    AllocConsole();
    PipeToSTDOut();
#endif

    Phoenix::Window::Instance()->Initialize(hInstance, "Phoenix (D3D11)", 800, 600);// , Resize, MouseClick, MouseDrag, MouseWheel);

    Phoenix::Window::Instance()->mResizeEvent += Phoenix::Window::ResizeEvent_t::Callback(&Resize);
    Phoenix::Window::Instance()->mClickEvent += Phoenix::Window::MouseEvent_t::Callback(&MouseClick);
    Phoenix::Window::Instance()->mDragEvent += Phoenix::Window::MouseEvent_t::Callback(&MouseDrag);
    Phoenix::Window::Instance()->mMouseWheelEvent += Phoenix::Window::MouseEvent_t::Callback(&MouseWheel);
    Phoenix::Window::Instance()->mMouseMoveEvent += Phoenix::Window::MouseEvent_t::Callback(&MouseMove);

    Phoenix::D3D11Manager::Instance()->Initialize();
    Phoenix::D3D11LineRenderer::Instance()->Initialize();
    Phoenix::D3D11MeshRenderer::Instance()->Initialize();
    Phoenix::D3D11imgui::Instance()->Initialize();
    Phoenix::Console::Instance()->Initialize();

    std::vector<Phoenix::Scenario*> scenarios = {new Phoenix::Visualize()};
    std::vector<stdstr_t> scenarioNames;
    for (auto sc : scenarios)
    {
        sc->Initialize();
        scenarioNames.push_back(sc->Name());
    }
    uint32_t currentScenarioIndex = 0;
    currentScenario = scenarios[currentScenarioIndex];//get current scenario

    //Show calls resize, calls scenario resize, calls camera perspetive (we need a valid scenario)
    Phoenix::Window::Instance()->Show(nShowCmd);

    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));
    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            Phoenix::D3D11Manager::Instance()->BeginRender();
            Phoenix::D3D11imgui::Instance()->BeginRender();

            //All other render
            currentScenarioIndex = Phoenix::D3D11imgui::Instance()->DrawComboBox("Scenarios", scenarioNames, currentScenarioIndex);
            currentScenario = scenarios[currentScenarioIndex];//get current scenario
            currentScenario->Run();

            Phoenix::D3D11LineRenderer::Instance()->Render(currentScenario->GetMainCamera());
            Phoenix::D3D11MeshRenderer::Instance()->Render(currentScenario->GetMainCamera());

            Phoenix::Console::Instance()->Draw();

            Phoenix::D3D11imgui::Instance()->EndRender();
            Phoenix::D3D11Manager::Instance()->EndRender();
        }
    }

    //delete scenarios
    for (auto sc : scenarios)
        delete sc;
    scenarios.clear();

    //cleanup
    Phoenix::D3D11imgui::Instance()->Shutdown();
    Phoenix::D3D11LineRenderer::Instance()->Shutdown();
    Phoenix::D3D11MeshRenderer::Instance()->Shutdown();
    Phoenix::D3D11Manager::Instance()->Shutdown();
}
 