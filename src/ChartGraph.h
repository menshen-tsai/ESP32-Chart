#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Time.h"

#ifndef _CHARTGRAPH_
#define _CHARTGRAPH_

#define max_readings   200
#define xticks        5                // 5 x-axis division markers
#define yticks        5                // 5 y-axis division markers
#define xGraphOffset  100
#define UNIT_LEN	  10

struct DataPoint {
  float    data;
  time_t   timeStamp;
};
class ChartGraph {
  public:
    ChartGraph(Adafruit_ILI9341 *tft,int x_pos, int y_pos, int width, int height, int Y1Min, int Y1Max, String title, boolean auto_scale, boolean barchart_mode, int graph_colour) ;
    ChartGraph();
    void begin();
//    void Draw();
//    void DrawGraph(int x_pos, int y_pos, int width, int height, int Y1Min, int Y1Max, String title, float DataArray[], boolean auto_scale, boolean barchart_mode, int graph_colour) ;
//    void DrawGraph(float DataArray[]) ;
    void UpdateGraph();
    void GraphColor(int);
    void BackgroundColor(int) ;
    void setUnit(char*);
    void addData(float);
    void addData(int16_t);
  private:
    uint16_t _index;
    uint16_t _max_readings;
    boolean  _autoScale;
    uint16_t _graphColor;
    uint16_t _backgroundColor;
    uint16_t _xPos, _yPos, _width, _height;
    uint16_t _y1Max;
    uint16_t _y1Min;    
    String   _Title;
    boolean  _barchartMode;
    float    _DataArray[max_readings];
    //float _Data[max_readings];
    struct DataPoint _Data[max_readings];
    int _Old_x2[max_readings+1];
    int _Old_y2[max_readings+1];
    long _xScale[6];
    uint32_t _timeTag;
    uint8_t _xScaleIndex;
    struct DataPoint  _maxValue, _minValue;
    struct DataPoint  _recentValue;
    char _unit[UNIT_LEN];
    
    
    Adafruit_ILI9341* _tft;
};
#endif
