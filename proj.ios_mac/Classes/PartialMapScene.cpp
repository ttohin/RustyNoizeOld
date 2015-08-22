#include "PartialMapScene.h"
#include "cocostudio/DictionaryHelper.h"
#include "UIButton.h"

static const int kSize = 512;

USING_NS_CC;
using namespace cocostudio;

void Mix(const PartialMapScene::FloatColor& a, const PartialMapScene::FloatColor& b, float proportion, PartialMapScene::FloatColor& c)
{
  c.r = a.r * proportion + b.r * (1.f - proportion);
  c.g = a.g * proportion + b.g * (1.f - proportion);
  c.b = a.b * proportion + b.b * (1.f - proportion);
}

void WriteColor(unsigned char* buf, const PartialMapScene::FloatColor& color)
{
  buf[0] = int(color.r * 255) % 256;
  buf[1] = int(color.g * 255) % 256;
  buf[2] = int(color.b * 255) % 256;
  buf[3] = 255;
}

Scene* PartialMapScene::createScene()
{
  auto scene = Scene::create();
  auto layer = PartialMapScene::create();
  scene->addChild(layer);
  return scene;
}

bool PartialMapScene::init()
{
  if ( !Layer::init() )
  {
    return false;
  }
  
  const int w = kSize;
  const int h = kSize;
  
  auto s = Director::getInstance()->getWinSize();
  
  m_menu = LabelTTF::create("Press left and right mouse buttons to change the world.\n"
                                 "Keys:\n"
                                 "R - restart\n"
                                 "H - help\n"
                                 "Q - exit\n\n"
                                 "ttohin (2015)\n"
                                 "thanks to hintertur for colors."
                                 ,
                                 "Marker Felt",
                                 32,
                                 s,
                                 TextHAlignment::LEFT,
                                 TextVAlignment::BOTTOM);
  
  m_menu->setPosition(Vec2(10 + s.width / 2, s.height / 2));
  
  addChild(m_menu, 999);
  ShowMenu();
  
  m_imageBuffer = std::shared_ptr<unsigned char>(new unsigned char[w * h * 4]);
  CreateGenerator();
  
  m_image = new Image::Image();
  m_image->initWithRawData(m_imageBuffer.get(), w * h * 4, w, h, 8);
  m_image->autorelease();
  
  m_texture = new Texture2D();
  m_texture->initWithImage(m_image);
  m_texture->autorelease();
  
  m_debugView = Sprite::createWithTexture(m_texture);
  //  m_debugView->setAnchorPoint({0, 0});
  Size viewSize = Director::getInstance()->getVisibleSize();
  m_debugView->setScale(AspectToFill({kSize, kSize}, viewSize));
  m_debugView->setPosition({viewSize.width/2.f, viewSize.height/2.f});
  
  addChild(m_debugView);
  

  CacheMapSize();
  
  
  auto keyboardListener = EventListenerKeyboard::create();
  keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event* event)
  {
    if (keyCode == EventKeyboard::KeyCode::KEY_R)
    {
      this->CreateGenerator();
    }
    else if (keyCode == EventKeyboard::KeyCode::KEY_H)
    {
      this->ShowMenu();
    }
    else if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE)
    {
      if (m_menu->isVisible())
      {
        HideMenu();
      }
      else
      {
        ShowMenu();
      }
    }
    else if (keyCode == EventKeyboard::KeyCode::KEY_Q)
    {
      this->Exit();
    }
  };
  
  _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);
  
  auto mouseListener = EventListenerMouse::create();

  mouseListener->onMouseMove = [this](Event* event) {
    
    if (m_mouseButtonCounter == 0) {
      return;
    }
    
    auto me = static_cast<EventMouse*>(event);
    m_isLeftMouseButton = me->getMouseButton() == MOUSE_BUTTON_LEFT;
    m_mousePos.reset(new cocos2d::Vec2(me->getLocation()));
  };
  
  mouseListener->onMouseUp = [this](Event* event) {
    
    auto me = static_cast<EventMouse*>(event);
    
    if (me->getMouseButton() == MOUSE_BUTTON_LEFT ||
        me->getMouseButton() == MOUSE_BUTTON_RIGHT)
    {
      if (m_mouseButtonCounter > 0)
        m_mouseButtonCounter -= 1;
      
      m_isLeftMouseButton = me->getMouseButton() != MOUSE_BUTTON_LEFT;
    }
    
    if (m_mouseButtonCounter == 0)
    {
      m_mousePos.reset();
    }
  };
  mouseListener->onMouseDown = [this](Event* event) {
    
    HideMenu();
    
    auto me = static_cast<EventMouse*>(event);
    
    if (me->getMouseButton() == MOUSE_BUTTON_LEFT ||
        me->getMouseButton() == MOUSE_BUTTON_RIGHT)
    {
      m_mouseButtonCounter += 1;
    }
    
    if (m_mouseButtonCounter != 0)
    {
      m_isLeftMouseButton = me->getMouseButton() == MOUSE_BUTTON_LEFT;
      m_mousePos.reset(new cocos2d::Vec2(me->getLocation()));
    }
  };
  
  _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
  
  schedule(schedule_selector(PartialMapScene::timerForUpdate));
  
  return true;
}

void PartialMapScene::CreateGenerator()
{
  FloatColor black1 = {27, 5, 0};
  FloatColor black2 = {27, 46, 42};
  m_blackColor = std::rand() % 2 ? black1 : black2;

  std::vector<FloatColor> array = {
      {37, 105, 121}
    , {157, 117, 68}
    , {154, 192, 203}
    , {117, 13, 0}
    , {255, 255, 255}
  };
  
  int index1 = std::rand() % array.size();
  int index2 = index1;
  while (index1 == index2) {
    index2 = std::rand() % array.size();
  }
  m_Color1 = array[index1];
  m_Color2 = array[index2];
  
  m_frameNumber = 0;
  m_noizeMode = NoizeMode(std::rand() % NoizeModeNumber);
  m_noizeLevel = 1.2 * std::rand() / (float)RAND_MAX;
  
  const int w = kSize;
  const int h = kSize;
  
  m_generator = std::make_shared<komorki::DiamondSquareGenerator>(w, h, 20.f, -0.1f, true);
  m_generator->Generate(nullptr);
}

void PartialMapScene::ForEach(float dt, int x, int y, float& value)
{
  if (x < m_xMin || x >= m_xMax || y < m_yMin || y >= m_yMax) {
    return;
  }
  
  ProcessMouse(dt, x, y, value);
  
  value += 0.5 * dt;
  if (value > 15.f)
  {
    value -= 15.f;
  }
  
  float diff = dt * m_noizeLevel * std::rand() / (float)RAND_MAX;
  diff *= ProcessNoize(x, y);
  value += diff;
  
  Draw(x, y, value);
}

float PartialMapScene::ProcessNoize(int x, int y)
{
  auto circleDistance = [&](int x, int y)
  {
    float xRatio = float(x - kSize/2)/float(kSize/2);
    float yRatio = float(y - kSize/2)/float(kSize/2);
    float distanceRatio = sqrtf(xRatio*xRatio + yRatio*yRatio);
    return distanceRatio;
  };
  
  auto linearRatio = [&](int x, int y, float xVal, float yVal)
  {
    float xRatio = float(kSize)/float(kSize);
    float yRatio = float(kSize)/float(kSize);
    return xRatio * xVal+ yRatio * yVal;
  };
  
  switch (m_noizeMode)
  {
    case NoizeModeCircle:
      return circleDistance(x, y);
    case NoizeModeCircleInner:
      return 1.f - circleDistance(x, y);
    case NoizeModeBottom:
      return linearRatio(x, y, 0, 1);
    case NoizeModeTop:
      return 1.f - linearRatio(x, y, 0, 1);
    case NoizeModeLeft:
      return 1.f - linearRatio(x, y, 1, 0);
    case NoizeModeRigth:
      return linearRatio(x, y, 1, 0);
    case NoizeModeEverywhere:
      return 1.f;
    case NoizeModeNone:
      return 0.f;
    case NoizeModeNumber:
      return 0.f;
  }
}

void PartialMapScene::ProcessMouse(float dt, int x, int y, float& value)
{
  if (m_mousePos == nullptr) {
    return;
  }
  
  Size viewSize = Director::getInstance()->getVisibleSize();
  int wDiff = kSize * m_debugView->getScale() - viewSize.width;
  int hDiff = kSize * m_debugView->getScale() - viewSize.height;
  
  int mousePosX = (m_mousePos->x + wDiff/2) / m_debugView->getScale();
  int mousePosY = (m_mousePos->y + hDiff/2) / m_debugView->getScale();
  
  float xRatio = float(x - mousePosX);
  float yRatio = float(y - mousePosY);
  float distance = (xRatio*xRatio + yRatio*yRatio) /  (2 *kSize*kSize);
  
  const float kThreshold = 0.1;
  if (distance > kThreshold) {
    return;
  }
  
  distance = distance * (1.f / kThreshold);
  float distanceRatio = 1.f - distance;
  distanceRatio = distanceRatio * distanceRatio;
  distanceRatio = 3 * dt * distanceRatio;
  
  if (m_isLeftMouseButton)
    value += distanceRatio;
  else
    value -= distanceRatio;
}

void PartialMapScene::Draw(int x, int y, float value)
{
  const float kScale = 0.5f;
  value *= kScale;
  float normilizeValue = value;
  if (value > 1.0)
  {
    normilizeValue = value - int(value);
  }
  
  int numberOfOverflows = int(value);
  float overflowRatio = float(numberOfOverflows)/ ( 15.f * kScale);
  
  if (overflowRatio >= 1.f)
  {
    overflowRatio = overflowRatio - int(overflowRatio);
    overflowRatio = overflowRatio* 0.5 + 0.5;
  }
  
  float v1 = (1.0f - overflowRatio * overflowRatio) * normilizeValue;
  float v2 = normilizeValue * 0.1 + overflowRatio * overflowRatio * 0.9;
  
  FloatColor colorMox;
  Mix(m_Color1, m_Color2, v2, colorMox);
  
  FloatColor blackMix;
  Mix(m_blackColor, colorMox, v1, blackMix);
  
  WriteColor(m_imageBuffer.get() + (x + y*kSize) * 4, blackMix);
}

void PartialMapScene::CacheMapSize()
{
  Size viewSize = Director::getInstance()->getVisibleSize();
  int wDiff = kSize * m_debugView->getScale() - viewSize.width;
  int hDiff = kSize * m_debugView->getScale() - viewSize.height;
  
  wDiff = (wDiff) / m_debugView->getScale();
  hDiff = (hDiff) / m_debugView->getScale();
  
  m_xMin = wDiff / 2;
  m_xMax = kSize - wDiff / 2;
  m_yMin = hDiff / 2;
  m_yMax = kSize - hDiff / 2;
}


void PartialMapScene::Exit()
{
  exit(0);
}

void PartialMapScene::ShowMenu()
{
  unschedule("hide");
  m_menu->setOpacity(255);
  m_menu->setVisible(true);

  schedule([&](float dt)
           {
             HideMenu();
           }, 5, "hide");
}

void PartialMapScene::HideMenu()
{
  if (!m_menu->isVisible()) {
    return;
  }
  
  unschedule("hide");
  m_menu->stopAllActions();
  auto fadeOut = FadeOut::create(0.5);
  auto seq = Sequence::create(fadeOut, Hide::create(), NULL);
  m_menu->runAction(seq);
}


void PartialMapScene::visit(cocos2d::Renderer *renderer, const cocos2d::Mat4 &parentTransform, uint32_t parentFlags)
{
  Size viewSize = Director::getInstance()->getVisibleSize();
  Size contentSize = getContentSize();
  
  if (!viewSize.equals(contentSize))
  {
    // before we visit we are going to set our size to the visible size.
    setContentSize(Director::getInstance()->getVisibleSize());
    
    m_debugView->setScale(AspectToFill({kSize, kSize}, viewSize));
    m_debugView->setPosition({viewSize.width/2.f, viewSize.height/2.f});
    CacheMapSize();
  }
  
  Layer::visit(renderer, parentTransform, parentFlags);
}



void PartialMapScene::timerForUpdate(float dt)
{
  m_frameNumber += 1;
  
  const int w = kSize;
  const int h = kSize;
  
  using namespace std::placeholders;
  m_generator->ForEach(std::bind(&PartialMapScene::ForEach, this, dt, _1, _2, _3));
  
  m_texture = new Texture2D();
  m_texture->initWithData(m_imageBuffer.get(), w * h * 4, Texture2D::PixelFormat::RGBA8888, kSize, kSize, {kSize, kSize});
  m_texture->autorelease();
  
  m_debugView->setTexture(m_texture);
  
}

float PartialMapScene::AspectToFit(const Size& source, const Size& target)
{
  float wAspect = target.width/source.width;
  float hAspect = target.height/source.height;
  return MIN(wAspect, hAspect);
}

float PartialMapScene::AspectToFill(const Size& source, const Size& target)
{
  float wAspect = target.width/source.width;
  float hAspect = target.height/source.height;
  return MAX(wAspect, hAspect);
}

