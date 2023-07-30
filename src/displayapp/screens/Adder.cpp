#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Adder.h"
#include <algorithm> //fill

using namespace Pinetime::Applications::Screens;

Adder::Adder(Pinetime::Components::LittleVgl& lvgl) : lvgl{lvgl}{

  AppReady=false;

  TileBufferSize = TileSize*TileSize;
  TileBuffer = new lv_color_t[TileBufferSize];


  DisplayHeight = LV_VER_RES_MAX;
  DisplayWidth = LV_HOR_RES_MAX;

  FieldHeight = DisplayHeight/TileSize-2;
  FieldWidth = DisplayWidth/TileSize;
  FieldOffsetHorizontal = (DisplayWidth-FieldWidth*TileSize)/2;
  FieldOffsetVertical = (DisplayHeight-FieldHeight*TileSize)/2+1.5*TileSize;

  
  FieldSize = FieldWidth*FieldHeight;

  Field= new AdderField[FieldSize];

  unsigned int start_position = FieldWidth * FieldHeight/2 + FieldWidth/2 + 2;
  unsigned int body[]={start_position,start_position-1};
  AdderBody.assign(body,body+2);



  std::fill(TileBuffer, TileBuffer + TileBufferSize, LV_COLOR_WHITE);
  createLevel();
  
  Score = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(Score, "0000");
  lv_obj_align(Score, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -TileSize, 0);

  taskRefresh = lv_task_create(RefreshTaskCallback, AdderDelayInterval, LV_TASK_PRIO_MID, this);

 
}

Adder::~Adder() {
  delete[] Field;
  delete[] TileBuffer;
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}


void Adder::createLevel(){
    for(unsigned int i=0; i<FieldSize;i++){
        unsigned int x = i%FieldWidth;
        unsigned int y = i/FieldWidth;
        if(y==0 || y==FieldHeight-1|| x==0 || x==FieldWidth-1)
            Field[i]=BORDER;
        else
            Field[i]=BLANK;

    }
}

void Adder::GameOver(){
  return;
}

bool Adder::OnTouchEvent(Pinetime::Applications::TouchEvents event){
  switch(event){
    case TouchEvents::SwipeLeft: AdderDirection=-1; break;
    case TouchEvents::SwipeUp: AdderDirection=-FieldWidth; break;
    case TouchEvents::SwipeDown: AdderDirection=+FieldWidth;break;
    case TouchEvents::SwipeRight: AdderDirection=1; break;
  }

  return true;
}


MoveConsequence Adder::checkMove(){
    if(AdderBody.front()+AdderDirection<FieldSize){
        if(Field[AdderBody.front()+AdderDirection]==BLANK)
            return MOVE;
        if(Field[AdderBody.front()+AdderDirection]==FOOD)
            return EAT;
    }

    return DEATH;
}


void Adder::createFood(){
    unsigned int pos= rand()%((FieldWidth)*(FieldHeight-2));
    pos+=FieldWidth;

    if (pos%FieldWidth==0 ||pos%FieldWidth==1)
        pos+=1;

    Field[pos]=FOOD;
    updateSingleTile(pos%FieldWidth,pos/FieldWidth,LV_COLOR_GREEN);
}

void Adder::updatePosition(){

    Field[AdderBody.front()]=BODY;
    Field[AdderBody.back()]=BLANK;

    switch(checkMove()){

        case DEATH:GameOver();break;

        case EAT: AdderBody.push_front(AdderBody.front()+AdderDirection); 
                  createFood();
                  break;

        case MOVE:AdderBody.pop_back();
                  AdderBody.push_front(AdderBody.front()+AdderDirection);
                  break;
        
    }
    
}

void Adder::FullReDraw(){
    lv_area_t area;
    lv_color_t selectColor=LV_COLOR_BLACK;

  for(int x=0; x < FieldWidth; x++){
    for(int y=0; y < FieldHeight; y++){


      switch(Field[y*FieldWidth+x]){
            case BODY: selectColor= LV_COLOR_YELLOW;break;
            case BORDER: selectColor=LV_COLOR_WHITE;break;
            case FOOD: selectColor=LV_COLOR_GREEN;break;
            default: selectColor=LV_COLOR_BLACK;break;
        }
      std::fill(TileBuffer,TileBuffer+TileBufferSize,selectColor);
      lv_area_t area;
      area.x1 = x*TileSize+FieldOffsetHorizontal;
      area.y1 = y*TileSize+FieldOffsetVertical;
      area.x2 = area.x1 + TileSize - 1;
      area.y2 = area.y1 + TileSize- 1;
      lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
      lvgl.FlushDisplay(&area, TileBuffer);
    }
  }
}

void Adder::Refresh(){
    updateDisplay();
}

void Adder::updateSingleTile(unsigned int FieldPosX,unsigned int FieldPosY,lv_color_t Color){
  std::fill(TileBuffer,TileBuffer+TileBufferSize,Color);
  lv_area_t area;
  area.x1 = FieldPosX*TileSize+FieldOffsetHorizontal;
  area.y1 = FieldPosY*TileSize+FieldOffsetVertical;
  area.x2 = area.x1 + TileSize - 1;
  area.y2 = area.y1 + TileSize- 1;
  lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
  lvgl.FlushDisplay(&area, TileBuffer);

}

void Adder::updateDisplay(){
  updatePosition();
  if(!AppReady){
    FullReDraw();
    createFood();
    updateSingleTile(AdderBody.back()%FieldWidth,AdderBody.back()/FieldWidth,LV_COLOR_BLACK);
    AppReady=true;
  }else{
    updateSingleTile(AdderBody.front()%FieldWidth,AdderBody.front()/FieldWidth,LV_COLOR_YELLOW);
    updateSingleTile(AdderBody.back()%FieldWidth,AdderBody.back()/FieldWidth,LV_COLOR_BLACK); 
  }
}



