// Jim Bumgardner 2015
import processing.pdf.*;

boolean outputPDF = true;
boolean outputNumbers = true;
boolean addFrameHoles = true;
String outputFileName = "test_spiral_11_5";

float mmToPoints = 2.83464567;
float mmtoInches = 0.0393701;
float inchToPoints = 72;

int m = 100;                                    // NUMBER OF LEDS
float holeRad= (12/2.0)*mmToPoints;
float frameHoleRad = (1/32.0) * inchToPoints;
float frameHoleMargin = (1/4.0) * inchToPoints;
float frameWidth = 11.5*inchToPoints;   // Total size of print area is 11.75
float marginWidth = 0.125*inchToPoints;
int sz = int(frameWidth + marginWidth*2);
float cx = sz/2;
float cy = sz/2;


float rad = 0.93 * frameWidth/2; // radius of rosette
float dotRad = holeRad; // we want 30mm diameter holes
float phi = (sqrt(5)+1)/2 - 1; // golden ratio
float ga = phi*2*PI;           // golden angle

void settings() {
  size(sz, sz);
}

void setup()
{
  noLoop();
  colorMode(RGB,1);
  ellipseMode(RADIUS);
  smooth();
}

void draw()
{
  float maxArea = pow(rad,2)*PI;
  float maxRad = sqrt(maxArea/PI);
  float circArea = maxArea/m;
  float circRad = sqrt(circArea/PI);

  background(1);

  if (outputPDF)
     beginRecord(PDF, outputFileName + ".pdf");


  // Cuts
  strokeWeight(0.25);
  stroke(0);
  noFill();
  
  float leftMargin = marginWidth;
  float topMargin = marginWidth;

  ellipseMode(RADIUS);

  for(int i = 0; i < m; i++) {
    float a = i*ga; // angle
    float radcum = sqrt((circArea*(i+0.5))/PI);
    float x = cx + cos(a)*radcum;
    float y = cy + sin(a)*radcum;
    println((i+1) + ": " + a + " " + radcum/inchToPoints + " " + x/inchToPoints + " x " + y/inchToPoints);
    ellipse(x,y,dotRad,dotRad);
  }
  rect(leftMargin, topMargin, frameWidth, frameWidth);
  
  if (addFrameHoles) {
    ellipse(marginWidth+frameHoleMargin, marginWidth+frameHoleMargin, frameHoleRad, frameHoleRad);
    ellipse(marginWidth+frameWidth/2.0, marginWidth+frameHoleMargin, frameHoleRad, frameHoleRad);
    ellipse(marginWidth+frameWidth-frameHoleMargin, marginWidth+frameHoleMargin, frameHoleRad, frameHoleRad);
    ellipse(marginWidth+frameWidth-frameHoleMargin, marginWidth+frameWidth-frameHoleMargin, frameHoleRad, frameHoleRad);
    ellipse(marginWidth+frameHoleMargin, marginWidth+frameWidth-frameHoleMargin, frameHoleRad, frameHoleRad);
  }

  if (outputNumbers) {
    textAlign(CENTER,CENTER);
    fill(0);
    for(int i = 0; i < m; i++) {
      float a = i*ga; // angle
      float radcum = sqrt((circArea*(i+0.5))/PI);
      float x = cx + cos(a)*radcum;
      float y = cy + sin(a)*radcum;
      text(""+i,x,y);
    }
  }

  

 if (outputPDF) {
    endRecord();
    exit();
  }
  save(outputFileName + ".png");
  
  // Identity longest connection...
  int[] dotToLight = {
      0,35,15,45,34,   1,44,16,80,36,
      14,46,27,81,44,  17,72,33,2,48,
      26,79,43,13,71,  28,88,49,18,73,
      37,3,64,25,82,   50,12,70,32,89,
      60,19,78,42,4,   65,24,87,55,11,   
      74,38,94,61,20,  83,51,5,67,29,      // 50s
      90,59,10,77,41,  95,63,22,86,54,
      6,69,31,93,58,   9,84,52,98,66,
      23,91,56,7,75,   39,96,62,21,85,
      53,99,68,30,92,  57,8,76,40,97};



  
}
