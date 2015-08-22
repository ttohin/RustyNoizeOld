#ifndef __PARTIALMAP_SCENE_H__
#define __PARTIALMAP_SCENE_H__

#include <vector>

#include "cocos2d.h"
#include "UIButton.h"
#include "DiamondSquareGenerator.h"

class PartialMapScene : public cocos2d::Layer, cocos2d::TextFieldDelegate
{
public:
  static cocos2d::Scene* createScene();
  virtual bool init();
  CREATE_FUNC(PartialMapScene);
  
  virtual ~PartialMapScene(){}
  
  struct FloatColor
  {
    float r;
    float g;
    float b;
    FloatColor(int _r, int _g, int _b) : r(_r/255.f), g(_g/255.f), b(_b/255.f) {}
    FloatColor() : r(0), g(0), b(0) {}
  };
  
  enum NoizeMode
  {
    NoizeModeNone,
    NoizeModeCircle,
    NoizeModeCircleInner,
    NoizeModeLeft,
    NoizeModeRigth,
    NoizeModeTop,
    NoizeModeBottom,
    NoizeModeEverywhere,
    NoizeModeNumber
  };
  
private:

  NoizeMode m_noizeMode;
  int m_frameNumber;
  float m_noizeLevel;
  std::shared_ptr<unsigned char> m_imageBuffer;
  
  cocos2d::Sprite* m_debugView;

  FloatColor m_blackColor;
  FloatColor m_Color1;
  FloatColor m_Color2;
  
  Node* m_menu;
  
  int m_xMin;
  int m_xMax;
  int m_yMin;
  int m_yMax;

  std::shared_ptr<cocos2d::Vec2> m_mousePos;
  bool m_isLeftMouseButton;
  int m_mouseButtonCounter = 0;
  
  cocos2d::Image* m_image;
  cocos2d::Texture2D* m_texture;
  std::shared_ptr<komorki::DiamondSquareGenerator> m_generator;
  std::shared_ptr<komorki::DiamondSquareGenerator> m_mix;

  void timerForUpdate(float dt);

  void Exit();
  
  
  void ShowMenu();
  void HideMenu();
  void CreateGenerator();
  void ForEach(float dt, int x, int y, float& value);
  float ProcessNoize(int x, int y);
  void ProcessMouse(float dt, int x, int y, float& value);
  void Draw(int x, int y, float value);
  void CacheMapSize();

  float AspectToFill(const cocos2d::Size& source, const cocos2d::Size& target);
  float AspectToFit(const cocos2d::Size& source, const cocos2d::Size& target);
  
  std::vector<cocos2d::Touch*> zoomToches;
  
  void visit(cocos2d::Renderer *renderer, const cocos2d::Mat4 &parentTransform, uint32_t parentFlags);
};

#endif // __PARTIALMAP_SCENE_H__
