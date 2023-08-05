#define LV_MONOSERT
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Adder.h"
#include <cstdlib> //randr


using namespace Pinetime::Applications::Screens;

Adder::Adder(Pinetime::Components::LittleVgl& lvgl) : lvgl{lvgl}{

  AppReady=false;

  TileBufferSize = TileSize*TileSize;
  TileBuffer = new lv_color_t[TileBufferSize];


  DisplayHeight = LV_VER_RES_MAX;
  DisplayWidth = LV_HOR_RES_MAX;

  FieldHeight = DisplayHeight/TileSize-1;
  FieldWidth = DisplayWidth/TileSize;
  FieldOffsetHorizontal = (DisplayWidth-FieldWidth*TileSize)/2;
  FieldOffsetVertical = (DisplayHeight-FieldHeight*TileSize)/2 + TileSize/2;

  
  FieldSize = FieldWidth*FieldHeight;

  Field= new AdderField[FieldSize];

  InitBody();


  
  for(int ti=0; ti<TileBufferSize;ti++)
    TileBuffer[ti]=LV_COLOR_WHITE;

  createLevel();

  taskRefresh = lv_task_create(RefreshTaskCallback, AdderDelayInterval, LV_TASK_PRIO_MID, this);

 
}

Adder::~Adder() {
  delete[] Field;
  delete[] TileBuffer;
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Adder::InitBody(){
  AdderBody.clear();
  unsigned int start_position = FieldWidth * FieldHeight/2 + FieldWidth/2 + 2;
  unsigned int body[]={start_position,start_position-1};
  AdderBody.assign(body,body+2);
}
void Adder::createLevel(){
    for(unsigned int i=0; i<FieldSize;i++){
        unsigned int x = i%FieldWidth;
        unsigned int y = i/FieldWidth;
        if(y==0 || y==FieldHeight-1|| x==0 || x==FieldWidth-1)
            Field[i]=SOLID;
        else
            Field[i]=BLANK;

    }
}

void Adder::GameOver(){
    unsigned int Digit[]={7,0,5,3};

    unsigned int Offset = FieldOffsetHorizontal>FieldOffsetVertical? FieldOffsetHorizontal: FieldOffsetVertical;
    for(int r= 3* Offset; r<DisplayWidth - 4*Offset;r+=16){
      for(int i=0; i<4;i++){
        for(int j=0; j<64; j++)
          DigitBuffer[63-j]=(DigitFont[Digit[i]][j/8] & 1<<j%8) ?LV_COLOR_WHITE:LV_COLOR_BLACK; //Bitmagic to map the font to an image array

        lv_area_t area;
        area.x1 = r + 8*i;
        area.y1 = r;
        area.x2 = area.x1 + 7;
        area.y2 = area.y1 + 7;
        lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
        lvgl.FlushDisplay(&area, DigitBuffer); 
      }
    }
    createLevel();
    AdderDirection=1;
    InitBody();
    AppReady=false;    
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

void Adder::updateScore(unsigned int Score){
    unsigned int Digit[]={0,Score%10,(Score%100-Score%10)/10,(Score-Score%100)/100};

    for(int i=0; i<4;i++){
      for(int j=0; j<64; j++)
        DigitBuffer[j]=(DigitFont[Digit[i]][j/8] & 1<<j%8) ?LV_COLOR_WHITE:LV_COLOR_BLACK; //Bitmagic to map the font to an image array

      lv_area_t area;
      area.x1 = DisplayWidth - 10 - 8*i;
      area.y1 = 2;
      area.x2 = area.x1 + 7;
      area.y2 = area.y1 + 7;
      lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
      lvgl.FlushDisplay(&area, DigitBuffer);
    }
    
      
}
void Adder::createFood(){
    Blanks.clear();

    for (int i = 0; i < FieldSize; ++i) 
        if (Field[i] == BLANK) 
            Blanks.push_back(i);

    unsigned int pos = rand()%Blanks.size();

    Field[Blanks[pos]]=FOOD;
    updateSingleTile(Blanks[pos]%FieldWidth,Blanks[pos]/FieldWidth,LV_COLOR_GREEN);
}

void Adder::updatePosition(){

    Field[AdderBody.front()]=BODY;
    Field[AdderBody.back()]=BLANK;

    switch(checkMove()){

        case DEATH:GameOver();break;

        case EAT: AdderBody.push_front(AdderBody.front()+AdderDirection); 
                  createFood();
                  updateScore(AdderBody.size()-2);
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
            case SOLID: selectColor=LV_COLOR_WHITE;break;
            case FOOD: selectColor=LV_COLOR_GREEN;break;
            default: selectColor=LV_COLOR_BLACK;break;
        }
      for(int ti=0; ti<TileBufferSize;ti++)
        TileBuffer[ti]=selectColor;
    
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
  for(int ti=0; ti<TileBufferSize;ti++)
    TileBuffer[ti]=Color;
    
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
    updateScore(0);
    AppReady=true;
  }else{
    updateSingleTile(AdderBody.front()%FieldWidth,AdderBody.front()/FieldWidth,LV_COLOR_YELLOW);
    updateSingleTile(AdderBody.back()%FieldWidth,AdderBody.back()/FieldWidth,LV_COLOR_BLACK); 
  }
}



