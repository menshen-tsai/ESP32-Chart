#include "ChartGraph.h"



ChartGraph::ChartGraph(Adafruit_ILI9341 *tft, int x_pos, int y_pos, int width, int height, int Y1Min, int Y1Max, String title, boolean autoscale, boolean barchart_mode, int graph_colour) {
  this->_xPos = x_pos;
  this->_yPos = y_pos;
  this->_width = width;
  this->_height = height;
  this->_y1Max = Y1Max;
  this->_y1Min = Y1Min;
  this->_Title = title;
  this->_autoScale = autoscale;
  this->_barchartMode = barchart_mode;
  this->_graphColor = graph_colour;
  this->_tft = tft;

}

ChartGraph::ChartGraph() {
  
}

void ChartGraph::begin() {
  //Graph the received data contained in an array
  // Draw the graph outline
  _maxValue.data = -9999;
  _minValue.data = 9999;
  _index = 0;
  for(int i=0; i<max_readings; i++)
     _Data[i].data = 0.0;
  _tft->fillRect(_xPos+xGraphOffset,_yPos,_width+2,_height+3,ILI9341_BLACK);  
  _tft->drawRect(_xPos+xGraphOffset,_yPos,_width+2,_height+3,ILI9341_WHITE);

  _tft->setTextSize(2);
  _tft->setTextColor(_graphColor, _backgroundColor);
  _tft->setCursor(_xPos+xGraphOffset + (_width - _Title.length()*12)/2,_yPos-20); // 12 pixels per char assumed at size 2 (10+2 pixels)
  _tft->print(_Title);
  _tft->setTextSize(1);
}


void ChartGraph::UpdateGraph() {
  #define auto_scale_major_tick 5 // Sets the autoscale increment, so axis steps up in units of e.g. 5
  
  int maxYscale = -9999;
  int minYscale = 9999;
  int x1,y1,x2,y2;
  int ind;
  char buf[10];

  _tft->setTextColor(ILI9341_YELLOW, _backgroundColor);
  _tft->setTextSize(2);
  _tft->setCursor(_xPos,_yPos + 30);
  _tft->print(_Data[(_index-1+max_readings)%max_readings].data,1);
  _tft->print(_unit);            //Deg-C symbol


  _tft->setTextSize(1);
  // Now display max and min temperature readings
  _tft->setTextColor(ILI9341_RED, _backgroundColor);
  _tft->setCursor(_xPos,_yPos+10);
  _tft->print(_maxValue.data,1);
  _tft->print(_unit);            //Deg-C symbol
  _tft->print(" max");

  _tft->setTextColor(ILI9341_GREEN, _backgroundColor);
  _tft->setCursor(_xPos,_yPos+55);
  _tft->print(_minValue.data,1);
  _tft->print(_unit);            //Deg-C symbol
  _tft->print(" min");
  
  if (_autoScale) {
    for (int i=0; i < max_readings; i++ )  {
      if (maxYscale <= _Data[i].data) 
        maxYscale = _Data[i].data;
      if (minYscale >= _Data[i].data) 
        minYscale = _Data[i].data;  
    }
    maxYscale = ((maxYscale + auto_scale_major_tick + 2) / auto_scale_major_tick) * auto_scale_major_tick; // Auto scale the graph and round to the nearest value defined, default was Y1Max
    minYscale = (((minYscale - auto_scale_major_tick - 2) / auto_scale_major_tick)+1) * auto_scale_major_tick; // Auto scale the graph and round to the nearest value defined, default was Y1Max    
////    if (maxYscale < _y1Max) 
      _y1Max = maxYscale; 
      _y1Min = minYscale;
  }

  // Draw the data
  for(int gx = 1; gx <= max_readings; gx++){
    x1 = _xPos+xGraphOffset + gx * _width/max_readings; 
    y1 = _yPos + _height;
    x2 = _xPos+xGraphOffset + gx * _width/max_readings; // max_readings is the global variable that sets the maximum data that can be plotted 
    ind = _index - 1 + gx;
    ind = (ind % max_readings);
////    y2 = _yPos + _height - constrain(_Data[ind].data,0,_y1Max) * _height / _y1Max + 1;
    y2 = _yPos + _height - (constrain(_Data[ind].data,_y1Min,_y1Max) - _y1Min) * _height / (_y1Max-_y1Min) + 1;
    if (_barchartMode) {
      if (Old_x2[gx] != 0 && Old_y2[gx] != 0)
        _tft->drawLine(x1,y1,Old_x2[gx],Old_y2[gx],ILI9341_BLACK);
      _tft->drawLine(x1,y1,x2,y2,_graphColor);
    } else {
      if (Old_x2[gx] != 0 && Old_y2[gx] != 0) {
        _tft->drawPixel(Old_x2[gx],Old_y2[gx],ILI9341_BLACK);
        _tft->drawPixel(Old_x2[gx],Old_y2[gx]-1,ILI9341_BLACK); // Make the line a double pixel height to emphasise it, -1 makes the graph data go up!
      }
      _tft->drawPixel(x2,y2,_graphColor);
      _tft->drawPixel(x2,y2-1,_graphColor); // Make the line a double pixel height to emphasise it, -1 makes the graph data go up!
    }
    Old_x2[gx] = x2;
    Old_y2[gx] = y2;    
  }

  //Draw the X-axis scale
  _tft->setTextSize(1);
  _tft->setTextColor(ILI9341_YELLOW, _backgroundColor);
  for (int spacing = 0; spacing <= xticks; spacing++) {
    #define number_of_ydashes 12
    for (int j=0; j < number_of_ydashes; j++){ // Draw dashed graph grid lines
      if (spacing < xticks) {
        _tft->drawFastVLine(_xPos+xGraphOffset+(_width*spacing/xticks), (_yPos+1+j*_height/number_of_ydashes), _height/(2*number_of_ydashes), ILI9341_WHITE);
      }
    }
    if ((spacing == 1) || (spacing == 3) || (spacing == 5)) {
      if (spacing == 5) {
        _tft->setCursor((_xPos+xGraphOffset+_width/xticks*spacing)-35,_yPos+_height+5);
      } else {
        _tft->setCursor((_xPos+xGraphOffset+_width/xticks*spacing)-18,_yPos+_height+5);
      }

    int x_value = (ind + spacing*(max_readings/xticks)) % max_readings;
    int hh, mm, ss;
    time_t t0;
    struct tm *timeInfo;
      
    t0 = _Data[x_value].timeStamp;
    timeInfo = localtime(&t0);
    ss = timeInfo->tm_sec;  //(_Data[x_value].ts/1000);
    mm = timeInfo->tm_min;    //ss / 60;
    hh = timeInfo->tm_hour;   //mm / 60;
    sprintf(buf, "%2d:%02d:%02d", hh, mm, ss);
    _tft->print(buf);
    }
  }
  

  
  //Draw the Y-axis scale
  for (int spacing = 0; spacing <= yticks; spacing++) {
    #define number_of_xdashes 40
    for (int j=0; j < number_of_xdashes; j++){ // Draw dashed graph grid lines
      if (spacing < yticks) 
        _tft->drawFastHLine((_xPos+xGraphOffset+1+j*_width/number_of_xdashes),_yPos+(_height*spacing/yticks),_width/(2*number_of_xdashes),ILI9341_WHITE);
    }
    _tft->setTextSize(1);
    _tft->setTextColor(ILI9341_YELLOW, _backgroundColor);
    _tft->setCursor((_xPos+xGraphOffset-20),_yPos+_height*spacing/yticks-3);
//    _tft->print(_y1Max - _y1Max / yticks * spacing);
    _tft->print(_y1Max - (_y1Max - _y1Min) / yticks * spacing);    
// Chars are too big.
//    sprintf(buf, "%d", _y1Max - (_y1Max - _y1Min) / yticks * spacing);
//    _tft->drawRightString(buf, (_xPos+xGraphOffset-20), _yPos+_height*spacing/yticks-3, 2);
  }
}

void ChartGraph::BackgroundColor(int color) {
  this->_backgroundColor = color;
}
void ChartGraph::GraphColor(int color) {
  this->_graphColor = color;
}



void ChartGraph::addData(float d) {
  time_t t0;
  time(&t0);
  _Data[_index].data = d;
  _Data[_index].timeStamp = t0;
  _index++;
  if (_index >= max_readings)
    _index = 0;

  if (d > _maxValue.data) {
    _maxValue.data = d;
    _maxValue.timeStamp = t0;
  }
  if (d < _minValue.data) {
    _minValue.data = d;
    _minValue.timeStamp = t0;
  }
    
}

void ChartGraph::addData(int16_t d) {
  
}

void ChartGraph::setUnit(char* u) {
  for (int i=0; i<UNIT_LEN; i++) {
    _unit[i] = u[i];
  }
}


