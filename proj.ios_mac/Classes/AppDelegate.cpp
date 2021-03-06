#include "AppDelegate.h"

#include <time.h>
#include <stdlib.h>
#include "PartialMapScene.h"

USING_NS_CC;

AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate()
{
}

bool AppDelegate::applicationDidFinishLaunching()
{
  timeval tval;
  gettimeofday(&tval, NULL);
  srand(tval.tv_usec);

  
//  srand(0);
  auto director = Director::getInstance();
  auto glview = director->getOpenGLView();
  if(!glview)
  {
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
    glview = GLViewImpl::createWithFullScreen("rust 0.1.1");
#else
    glview = GLViewImpl::createWithRect("rust 0.1.1", Rect(0, 0, 1024, 768), 1.0);
#endif
    
    director->setOpenGLView(glview);
    
    Size glViewSize = glview->getFrameSize();
    glview->setFrameSize(glViewSize.width, glViewSize.height);
  }
  
//  director->setDisplayStats(true);
  
  director->setAnimationInterval(1.0 / 60);
  auto scene = PartialMapScene::createScene();
  director->runWithScene(scene);
  
  return true;
}

void AppDelegate::applicationDidEnterBackground()
{
    Director::getInstance()->stopAnimation();
}

void AppDelegate::applicationWillEnterForeground()
{
    Director::getInstance()->startAnimation();
}
