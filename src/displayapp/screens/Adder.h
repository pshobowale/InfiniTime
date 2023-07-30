#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>
#include <list>
#include <random>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      enum AdderField{UNDEFINED,BLANK,BORDER,BODY,FOOD};
      enum MoveConsequence{DEATH=0,EAT,MOVE};

      class Adder : public Screen {

      public:
        Adder(Pinetime::Components::LittleVgl& lvgl);
        ~Adder() override;
        void Refresh() override;
        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;
        //bool OnButtonPushed()override{FullReDraw();return true;};
;
      private: // Private members and methods
        const unsigned int TileSize=9;
        const unsigned int AdderDelayInterval=150;
        bool AppReady;
        unsigned int DisplayHeight;
        unsigned int DisplayWidth;

        unsigned int FieldWidth;
        unsigned int FieldHeight;
        unsigned int FieldSize;
        unsigned int FieldOffsetHorizontal;
        unsigned int FieldOffsetVertical;

        AdderField* Field;
        std::list<unsigned int> AdderBody;

        int AdderDirection=1;
        char lastKey=0;
        

        unsigned int TileBufferSize;
        lv_color_t* TileBuffer;
        Pinetime::Components::LittleVgl& lvgl;
        lv_task_t* taskRefresh;
        lv_obj_t* Score;

        void createFood();

        MoveConsequence checkMove();
        void createLevel();
        void FullReDraw();
        void updateDisplay();
        void updateSingleTile(unsigned int FieldPosX,unsigned int FieldPosY,lv_color_t Color);
        void updatePosition();
        void GameOver();


      };
    }
  }
}

