/* COPYRIGHT (c) 2019 DIVO Systems, Inc.
 * Developed by DIVO Systems, Inc.   http://www.divo.com */

#include "MegunoLink.h"

#include "common.h"

static TimePlot odPlot;
static char TubeName[NUM_TUBES][7] = 
  {"Tube_1", "Tube_2", "Tube_3", "Tube_4", "Tube_5", "Tube_6"};
static Plot::Colors TubeColor[NUM_TUBES] = 
  {Plot::Magenta, Plot::Green, Plot::Blue, Plot::Red, Plot::Cyan, Plot::Black};

void POD_Init(void)
{
  int i;

  odPlot.SetTitle("Optical Density");
  odPlot.SetXlabel("Time");
  odPlot.SetYlabel("OD");
  for (i = 0; i < NUM_TUBES; i++)
    odPlot.SetSeriesProperties(TubeName[i], TubeColor[i], Plot::Solid, 2, Plot::Star);
}


void POD_SendData(int od_index, double data)
{
  odPlot.SendData(TubeName[od_index], data);
}
