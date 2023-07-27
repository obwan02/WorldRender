/* Creation Date: 5/7/2023
 * Creator: obwan02
 * ========================
 *
 * This file should be the main compilation point
 * for macOS
 *
 */
#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#include "game/graphics/metal_api.h"
#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>
#include <stdio.h>

#include "game/graphics/api.h"

#include "game/game.cpp"
#include "game/graphics/metal_api.cpp"
#include "game/util/log.cpp"
#include "game/util/macOS_assert.cpp"

static wrld::Application *game;

class MTKViewDelegate : public MTK::ViewDelegate {
public:
  MTKViewDelegate(MTL::Device *pDevice);
  virtual ~MTKViewDelegate() override;
  virtual void drawInMTKView(MTK::View *pView) override;
};

class AppDelegate : public NS::ApplicationDelegate {
public:
  ~AppDelegate();

  NS::Menu *createMenuBar();

  virtual void applicationWillFinishLaunching(NS::Notification *pNotification) override;
  virtual void applicationDidFinishLaunching(NS::Notification *pNotification) override;
  virtual bool applicationShouldTerminateAfterLastWindowClosed(NS::Application *pSender) override;

private:
  NS::Window *_pWindow;
  MTK::View *_pMtkView;
  wrld::g::Device _device;
  MTKViewDelegate *_pViewDelegate = nullptr;
};

AppDelegate::~AppDelegate() {
  _pMtkView->release();
  _pWindow->release();
  delete _pViewDelegate;
}

NS::Menu *AppDelegate::createMenuBar() {
  using NS::StringEncoding::UTF8StringEncoding;

  NS::Menu *pMainMenu = NS::Menu::alloc()->init();
  NS::MenuItem *pAppMenuItem = NS::MenuItem::alloc()->init();
  NS::Menu *pAppMenu = NS::Menu::alloc()->init(NS::String::string("Appname", UTF8StringEncoding));

  NS::String *appName = NS::RunningApplication::currentApplication()->localizedName();
  NS::String *quitItemName = NS::String::string("Quit ", UTF8StringEncoding)->stringByAppendingString(appName);
  SEL quitCb = NS::MenuItem::registerActionCallback("appQuit", [](void *, SEL, const NS::Object *pSender) {
    auto pApp = NS::Application::sharedApplication();
    pApp->terminate(pSender);
  });

  NS::MenuItem *pAppQuitItem = pAppMenu->addItem(quitItemName, quitCb, NS::String::string("q", UTF8StringEncoding));
  pAppQuitItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);
  pAppMenuItem->setSubmenu(pAppMenu);

  NS::MenuItem *pWindowMenuItem = NS::MenuItem::alloc()->init();
  NS::Menu *pWindowMenu = NS::Menu::alloc()->init(NS::String::string("Window", UTF8StringEncoding));

  SEL closeWindowCb = NS::MenuItem::registerActionCallback("windowClose", [](void *, SEL, const NS::Object *) {
    auto pApp = NS::Application::sharedApplication();
    pApp->windows()->object<NS::Window>(0)->close();
  });
  NS::MenuItem *pCloseWindowItem = pWindowMenu->addItem(NS::String::string("Close Window", UTF8StringEncoding), closeWindowCb, NS::String::string("w", UTF8StringEncoding));
  pCloseWindowItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);

  pWindowMenuItem->setSubmenu(pWindowMenu);

  pMainMenu->addItem(pAppMenuItem);
  pMainMenu->addItem(pWindowMenuItem);

  pAppMenuItem->release();
  pWindowMenuItem->release();
  pAppMenu->release();
  pWindowMenu->release();

  return pMainMenu->autorelease();
}

void AppDelegate::applicationWillFinishLaunching(NS::Notification *pNotification) {
  NS::Menu *pMenu = createMenuBar();
  NS::Application *pApp = reinterpret_cast<NS::Application *>(pNotification->object());
  pApp->setMainMenu(pMenu);
  pApp->setActivationPolicy(NS::ActivationPolicy::ActivationPolicyRegular);
}

void AppDelegate::applicationDidFinishLaunching(NS::Notification *pNotification) {
  CGRect frame = (CGRect){{100.0, 100.0}, {512.0, 512.0}};

  _pWindow = NS::Window::alloc()->init(
      frame,
      NS::WindowStyleMaskClosable | NS::WindowStyleMaskTitled,
      NS::BackingStoreBuffered,
      false);

  _pMtkView = MTK::View::alloc()->init(frame, _device._metalDevice);
  _pMtkView->setColorPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
  _pMtkView->setClearColor(MTL::ClearColor::Make(0.0, 0.0, 0.0, 1.0));

  _pViewDelegate = new MTKViewDelegate(_device._metalDevice);
  _pMtkView->setDelegate(_pViewDelegate);

  _pWindow->setContentView(_pMtkView);
  _pWindow->setTitle(NS::String::string("macOS CPP World Render", NS::StringEncoding::UTF8StringEncoding));

  _pWindow->makeKeyAndOrderFront(nullptr);

  NS::Application *pApp = reinterpret_cast<NS::Application *>(pNotification->object());
  pApp->activateIgnoringOtherApps(true);
}

bool AppDelegate::applicationShouldTerminateAfterLastWindowClosed(NS::Application *pSender) {
  return true;
}

MTKViewDelegate::MTKViewDelegate(MTL::Device *pDevice) : MTK::ViewDelegate() {}
MTKViewDelegate::~MTKViewDelegate() {}

void MTKViewDelegate::drawInMTKView(MTK::View *pView) {
  game->update();
  game->render(wrld::g::PresentParams{
      ._metalView = pView,
  });
}

int main(int argc, char *argv[]) {
  NS::AutoreleasePool *pAutoreleasePool = NS::AutoreleasePool::alloc()->init();
  AppDelegate del;
  NS::Application *pSharedApplication = NS::Application::sharedApplication();
  pSharedApplication->setDelegate(&del);

  {
    static wrld::g::Device device{};
    static wrld::Application _app{&device};
    game = &_app;
  }
  pSharedApplication->run();

  pAutoreleasePool->release();

  return 0;
}
