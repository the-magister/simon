#ifdef LaserWipe_h
#define LaserWipe_h


void laserWipe(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData);
void laserWipeEdge(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData);

#endif

