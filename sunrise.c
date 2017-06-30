//
//  Sunrise.c
//  Lunar Foundation
//
//  Created by Michael Boich on 10/1/09.
//  Copyright 2009 Self. All rights reserved.
//  Ported from Objective C to Gnu C June 2017
//
#undef debug
#include "time.h"

#include "JulianDay.h"
#include "ViewingLocation.h"

#define kSun 1
#define kMoon 2

/*
void init_location(struct location *l){
  l->latitude = 34.04;		// Portola Valley, CA
  l->longitude = 118.52;
  l->viewing_date = time(NULL);
  l->gmt_offset = -7;
}
*/

void calcLunarAzimuth(double *azimuthResult, double *elevationResult, double *fullness, double *rightAscensionResult, double *declinationResult, time_t theDate,struct location theLocation){

  // -(void) calcLunarAzimuth: (double *) azimuthResult elevation: (double *) elevationResult illuminatedFraction:(double *) \
  //  fullness apRightAscension: (double *) rightAscensionResult  
  //  apDeclination: (double *) declinationResult forDate: (NSDate *) theDate {
	
  int table47A[60][6] = {	
    {0, 0, 1, 0, 6288774, -20905355},  
    {2, 0, -1, 0, 1274027, -3699111},
    {2, 0, 0, 0, 658314, -2955968}, 
    {0, 0, 2, 0, 213618, -569925}, 
    {0, 1, 0, 0, -185116, 48888},
    {0, 0, 0, 2, -114332, -3149},
    {2, 0, -2, 0, 58793, 246158},
    {2, -1, -1, 0, 57066, 152138},
    {2, 0, 1, 0, 53322, -170733},
    {2, -1, 0, 0, 45758, -204586},
    {0, 1, -1, 0, -40923, -129620},
    {1, 0, 0, 0, -34720, 108743},
    {0, 1, 1, 0, -30383, 104755},
    {2, 0, 0, -2, 15327, 10321},
    {0, 0, 1, 2, -12528, 0},
    {0, 0, 1, -2, 10980, 79661},
    {4, 0, -1, 0, 10675, -34782},
    {0, 0, 3, 0, 10034, -23210},
    {4, 0, -2, 0, 8548, -21636},
    {2, 1, -1, 0, -7888, 24208},
    {2, 1, 0, 0, -6766, 30824},
    {1, 0, -1, 0, -5163, -8379},
    {1, 1, 0, 0, 4987, -16675}, 
    {2, -1, 1, 0, 4036, -12831},
    {2, 0, 2, 0, 3994, -10445},
    {4, 0, 0, 0, 3861, -11650},
    {2, 0, -3, 0, 3665, 14403},
    {0, 1, -2, 0, -2689, -7003},
    {2, 0, -1, 2, -2602, 0},
    {2, -1, -2, 0, 2390, 10056},
    {1, 0, 1, 0, -2348, 6322},
    {2, -2, 0, 0, 2236, -9884},
    {0, 1, 2, 0, -2120, 5751},
    {0, 2, 0, 0 -2069, 0},
    {2, -2, -1, 0, 2048, -4950},
    {2, 0, 1, -2, -1773, 4130},
    {2, 0, 0, 2, -1595, 0},
    {4, -1, -1, 0, 1215, -3958},
    {0, 0, 2, 2, -1110, 0},
    {3, 0, -1, 0, -892, 3258},
    {2, 1, 1, 0, -810, 2616},
    {4, -1, -2, 0, 759, -1897},
    {0, 2, -1, 0, -713, -2117},
    {2, 2, -1, 0, -700, 2354},
    {2, 1, -2, 0, 691, 0},
    {2, -1, 0, -2, 596, 0},
    {4, 0, 1, 0, 549, -1423},
    {0, 0, 4, 0, 537, -1117},
    {4, -1, 0, 0, 520, -1571},
    {1, 0, -2, 0, -487, -1739},
    {2, 1, 0, -2 -399, 0},
    {0, 0, 2, -2, -381, -4421},
    {1, 1, 1, 0, 351, 0},
    {3, 0, -2, 0, -340, 0},
    {4, 0, -3, 0, 330, 0},
    {2, -1, 2, 0, 327, 0},
    {0, 2, 1, 0, -323, 1165},
    {1, 1, -1, 0, 299, 0},
    {2, 0, 3, 0, 294, 0},
    {2, 0, -1, -2, 0, 8752}
		
  };
	
  int table47B[60][5] = {
    {0, 0, 0, 1, 5128122},
    {0, 0, 1, 1, 280602},
    {0, 0, 1, -1, 277693},
    {2, 0, 0, -1, 173237},
    {2, 0, -1, 1, 55413},
    {2, 0, -1, -1, 46271},
    {2, 0, 0, 1, 32573},
    {0, 0, 2, 1, 17198},
    {2, 0, 1, -1, 9266},
    {0, 0, 2, -1, 8822},
    {2, -1, 0, -1, 8216},
    {2, 0, -2, -1, 4324},
    {2, 0, 1, 1, 4200},
    {2, 1, 0, -1, -3359},
    {2, -1, -1, 1, 2463},
    {2, -1, 0, -1, 2211},
    {2, -1, -1, -1, 2065},
    {0, 1, -1, -1, -1870},
    {4, 0, 0, -1, 1828},
    {0, 1, 0, 1, -1794},
    {0, 0, 0, 3, -1749},
    {0, 1, -1, 1, -1565},
    {1, 0, 0, 1, -1492},
    {0, 1, 1, 1, -1475},
    {0, 1, 1, -1, -1410},
    {0, 1, 0, -1, -1344},
    {1, 0, 0, -1, -1335},
    {0, 0, 3, 1, 1107},
    {4, 0, 0, -1, 1021},
    {4, 0, -1, 1, 833},
    {0, 0, 1, -3, 777},
    {4, 0, -2, 1, 671},
    {2, 0, 0, -3, 607},
    {2, 0, 2, -1, 596},
    {2, -1, 1, -1, 491},
    {2, 0, -2, 1, -451},
    {0, 0, 3, -1, 439},
    {2, 0, 2, 1, 422},
    {2, 0, -3, -1, 421},
    {2, 1, -1, 1, -366},
    {2, 1, 0, 1, -351},
    {4, 0, 0, 1, 331},
    {2, -1, 1, 1, 315},
    {2, -2, 0, -1, 302},
    {0, 0, 1, 3, -283},
    {2, 1, 1, -1, -229},
    {1, 1, 0, -1, 223}, 
    {1, 1, 0, 1, 223},
    {0, 1, -2, 1, -220},
    {2, 1, -1, -1, -220},
    {1, 0, 1, 1, -185},
    {2, -1, -2, -1, 181},
    {0, 1, 2, 1, -177},
    {4, 0, -2, -1, 176},
    {4, -1, -1, -1, 166},
    {1, 0, 1, -1, -164},
    {4, 0, 1, -1, 132},
    {1, 0, -1, -1, -119},
    {4, -1, 0, -1, 115},
    {2, -2, 0, 1, 107}
  };
	
	
	
  double theJulianDay = julianDay(theDate);
  #ifdef debug
  printf("%s -- %f\n",ctime(&theDate),theJulianDay);
  #endif
	
  double T = (theJulianDay - 2451545)/36525;		//Julian centuries since Epoch J2000.0
	
  // Moon's mean longitude, including constant term for the effect of light time:
  double	LPrime = T/65194000;
  LPrime = T*(LPrime + 1/538841);
  LPrime = T*(LPrime - 0.0015786);
  LPrime = T*(LPrime + 481267.88123421);
  LPrime = LPrime + 218.3164477;
  LPrime = reduce360(LPrime);
	
  //D is the mean elongation of the Moon:
  // calculate D = 297.8501921 + 445267.1114034T -0.00018819*t^2 + T^3/545868 - t^4/113065000 with Hoerner's rule
  double D = -T/113065000;
  D = T*(D + 1.0/545868);
  D = T*(D -.0018819);
  D = T*(D + 445267.1114034);
  D = D + 297.8501921;
  D = reduce360(D);
	
  //M s the mean anomaly of the Sun:
  double M = T/24490000;
  M = T*(M-0.0001536);
  M = T*(M+35999.0502909);
  M = M+357.5291092;
  M = reduce360(M);
	
  // MPrime is Moon's mean anomaly:
  double MPrime = T/14712000.0;
  MPrime = T*(MPrime + 1/69699.0);
  MPrime = T*(MPrime + 0.0087414);
  MPrime = T*(MPrime + 477198.8675055);
  MPrime = MPrime + 134.9633964;
  MPrime = reduce360(MPrime);
	
  //F is the Moon's argument of latitude:
  double F = T/863310000;
  F = T*(F - 1.0/3526000);
  F = T*(F - .0036539);
  F = T*(F + 483202.0175233);
  F = F + 93.2720950;
  F = reduce360(F);
	
  //Three magical quantities having to do with Jupiter, Venus, and flattening of Earth (angles, in degrees:);
  double A1 = reduce360(119.75 + 131.849*T);
  double A2 = reduce360(53.09 + 479264.290*T);
  double A3 = reduce360( 313.45 + 481266.484*T);
	
  // Now calculate the quantities sigmaL and sigmaR which each take the 60 periodic terms into account:
	
  double sigmaL = 0;
  double sigmaR = 0;
  double sigmaB = 0;
	
  double E = 1 - 0.002516*T - 0.0000074*T*T;
	
  // calc sigmaL, sigmaR:
   int row;
  for(row = 0; row < 60; row++){
    double coefficientForD = table47A[row][0];
    double coefficientForM = table47A[row][1];
    double coefficientForMPrime = table47A[row][2];
    double coefficientForF = table47A[row][3];
    double coefficientForSin = table47A[row][4];
    double coefficientForCos = table47A[row][5];
		
		
    double sinCosArg = 0;
    sinCosArg = coefficientForD * D;
    sinCosArg = sinCosArg + coefficientForM * M;
    sinCosArg = sinCosArg + coefficientForMPrime * MPrime;
    sinCosArg = sinCosArg	+ coefficientForF * F;
    sinCosArg = degToRad(sinCosArg);
		
    double sinAddend = coefficientForSin * sin(sinCosArg);
    double cosAddend = coefficientForCos * cos(sinCosArg);
		
    // if M is involved in the argument, multiply by E (or if 2M involved, by E^2):
    if (coefficientForM == 1.0 || coefficientForM == -1.0) {
      sinAddend = sinAddend * E;
      cosAddend = cosAddend * E;
    }
    else if(coefficientForM == 2.0 || coefficientForM == -2.0){
      sinAddend = sinAddend * E * E;
      cosAddend = cosAddend * E * E;
    }
		
    sigmaL = sigmaL + sinAddend;
    sigmaR = sigmaR + cosAddend;
  }
	
  // magic additions for influences of Venus, Jupiter, and the flattening of the earth:
  sigmaL = sigmaL + 3958.0 * sin(degToRad(A1)) + 1962*sin(degToRad(LPrime-F)) + 318*sin(degToRad(A2));
  //NSLog(@"sigmaL = %f",sigmaL);
  //NSLog(@"sigmaR = %f",sigmaR);
	
  //now calculate sigmaB, the correction for distance:
  for(row = 0; row < 60; row++){
    double coefficientForD = table47B[row][0];
    double coefficientForM = table47B[row][1];
    double coefficientForMPrime = table47B[row][2];
    double coefficientForF = table47B[row][3];
    double coefficientForSin = table47B[row][4];
		
		
    double sinCosArg = 0;
    sinCosArg = coefficientForD * D;
    sinCosArg = sinCosArg + coefficientForM * M;
    sinCosArg = sinCosArg + coefficientForMPrime * MPrime;
    sinCosArg = sinCosArg	+ coefficientForF * F;
    sinCosArg = degToRad(sinCosArg);
    double sinAddend = coefficientForSin * sin(sinCosArg);
		
    // if M is involved in the argument, multiply by E (or if 2M involved, by E^2):
    if (coefficientForM == 1.0 || coefficientForM == -1.0) {
      sinAddend = sinAddend * E;
    }
    else if(coefficientForM == 2.0 || coefficientForM == -2.0){
      sinAddend = sinAddend * E * E;
    }
		
    sigmaB = sigmaB + sinAddend;
  }
	
  //Adjustments to SigmaB:
  sigmaB = sigmaB - 2235 * sin(degToRad(LPrime)) + 382 * sin(degToRad(A3)) + 
    175 * sin(degToRad(A1 - F)) + 175 * sin(degToRad(A1 + F)) + 127 * sin(degToRad(LPrime - MPrime)) - 115 * sin(degToRad(LPrime + MPrime));
	
  // now calculate lambda (ecliptical longitude), beta (ecliptical latitude), delta (distance), and pi (parallax):
  // lambda is equal to LPrime - sigmaL/1000000:
  double lambda = LPrime + sigmaL/1000000;
	
  //beta = sigmaB/1000000:
  double beta = sigmaB/1000000;
	
  double distance = 385000.56 + sigmaR/1000;
	
  double piAngle = asin(6378.14/distance);
  piAngle = radToDeg(piAngle);
	
  //ignoring the nutation for the time being
	
  // transform from ecliptical coordinates to equatorial:
  double e2000 = 23.4392911;	 // obliquity of the ecliptic for epoch 2000.  Close enough for our purposes...
	
  //calculate the right ascension:
  double alpha = atan2((sin(degToRad(lambda))*cos(degToRad(e2000)) -tan(degToRad(beta))*sin(degToRad(e2000))), cos(degToRad(lambda)));
  alpha = radToDeg(alpha);
  alpha = reduce360(alpha);
	
  if (alpha < 0) {
    alpha = alpha + 360.0;
  }
	
  if (rightAscensionResult) {
    *rightAscensionResult = alpha;
  }
	
  //calculate the declination:
  double sinDelta = sin(degToRad(beta))*cos(degToRad(e2000))+cos(degToRad(beta))*sin(degToRad(e2000))*sin(degToRad(lambda));
  double delta = asin(sinDelta);
	
	
  delta = radToDeg(delta);
  delta = reduce360(delta);
	
  if (delta > 90) {
    delta = delta -360.0;
  }
	
  if (declinationResult) {
    *declinationResult = delta;
  }
	
  // now convert to local horizontal coordinates:
  double littleThetaZero = reduce360(littleThetaZeroInDegrees(theDate));  // sidereal time at Greenwich
	
  double H = littleThetaZero - theLocation.longitude - alpha;		// H = local hour angle
	
  double azimuth = atan2(sin(degToRad(H)), cos(degToRad(H))*sin(degToRad(theLocation.latitude))-tan(degToRad(delta))*cos(degToRad(theLocation.latitude)));
  azimuth = radToDeg(azimuth);
  azimuth = azimuth + 180;	// since astronomers measure from South, and I measure from North
  azimuth = reduce360(azimuth);
	
  double sinOfElevation = sin(degToRad(theLocation.latitude))*sin(degToRad(delta))+cos(degToRad(theLocation.latitude))*cos(degToRad(delta))*cos(degToRad(H));
  double elevation = radToDeg(asin(sinOfElevation));
	
	
  if (azimuthResult) {
    *azimuthResult = azimuth;
  }
	
  if (elevationResult) {
    *elevationResult = elevation;
  }
	
  // now throw in the "crude" version of the illuminated fraction calculation
  double i = 180 - D - 6.289*sin(degToRad(MPrime)) + 2.100 * sin(degToRad(M))-1.274 * sin(degToRad(2*D - MPrime))
    -0.658 * sin(degToRad(2*D)) - 0.214 * sin(2 * degToRad(MPrime)) - 0.110 * sin(degToRad(D));
	
  if (fullness) {
    *fullness = (1 + cos(degToRad(i)))/2;
  }
	
}
		
void calcSolarAzimuth(double *azimuthResult, double *elevationResult, double *rightAscensionResult, double *declinationResult, time_t theDate, struct location theLocation){
  // Calculate the azimuth and elevation of the sun for given date, location
  double the_julianDay = julianDay(theDate);
  double T = (the_julianDay - 2451545)/36525;			// Julian century from Epoch J2000.0
	
  // Compute geometric mean longitude of the Sun:
  double LZero = 280.46646 + 36000.76983*T + .0003032*T*T;
	
  // Compute mean anomaly of the Sun:
  double M = 357.52911 + 35999.050*T - .0001537*T*T;
	
  // eccentricity of the Earth;s orbit:
  double e = .016708634 - 0.000042037*T - .0000001267*T*T;
	
  // Sun's equation of the Center:
  double C = (1.914602 - 0.004817*T - .000014*T*T) * sin(degToRad(M))
    + (0.019993 - 0.000101*T) * sin(degToRad(2*M))
    + 0.000289 * sin(degToRad(3*M));
	
  double sunTrueLong = reduce360(LZero + C);
  double sunTrueAnomaly = reduce360(M + C);
	
  // convert from true long/anomaly to apparent long anomaly:
  double omega = reduce360(125.04 - 1934.136*T);
  double lambda = sunTrueLong - .00569 - .00478*sin(degToRad(omega));
	
  // now get almost home by calculating alpha and delta (right ascension and declination):
  double epsilon = 23.43929 - .01300417*T - 0.0000001638889*T*T + .0000005036111*T*T*T;
	
  // correct epsilon for apparent vs. mean:
  epsilon = epsilon + .00256*cos(degToRad(omega));
  double alpha = atan2(cos(degToRad(epsilon))*sin(degToRad(lambda)), cos(degToRad(lambda)));
  alpha = radToDeg(alpha);
  if (rightAscensionResult) {
    *rightAscensionResult = alpha;
  }

  double delta = asin(sin(degToRad(epsilon)*sin(degToRad(lambda))));
  delta = radToDeg(delta);
  if (declinationResult) {
    *declinationResult = delta;
  }
	

  // now convert to local horizontal coordinates:
  double littleThetaZero = reduce360(littleThetaZeroInDegrees(theDate));  // sidereal time at Greenwich
	
  double H = littleThetaZero - theLocation.longitude - alpha;		// H = local hour angle
	
  double azimuth = atan2(sin(degToRad(H)), cos(degToRad(H))*sin(degToRad(theLocation.latitude))-tan(degToRad(delta))*cos(degToRad(theLocation.latitude)));
  azimuth = radToDeg(azimuth);
  azimuth = azimuth + 180;	// since astronomers measure from South, and I measure from North
  azimuth = reduce360(azimuth);
	
  float sinOfElevation = sin(degToRad(theLocation.latitude))*sin(degToRad(delta))+cos(degToRad(theLocation.latitude))*cos(degToRad(delta))*cos(degToRad(H));
  float radArcSin = asin(sinOfElevation);
	
  float elevation = radToDeg(radArcSin);
	
  if (azimuthResult) {
    *azimuthResult = azimuth;
  }
  if (elevationResult) {
    *elevationResult = elevation;
  }
	
	
}
/*
  - (void) calcPathData{
  NSCalendar *gregorian =  [[NSCalendar alloc]
  initWithCalendarIdentifier:NSGregorianCalendar];
  NSTimeZone *localTimeZone = [NSTimeZone localTimeZone];
  [gregorian setTimeZone:localTimeZone];
	
  Lunar3AppDelegate *delegate = (id) [[UIApplication sharedApplication] delegate];
  NSDateComponents *dayMonthYear = [gregorian components:(NSDayCalendarUnit | NSMonthCalendarUnit | NSYearCalendarUnit)
  fromDate:delegate.selectedTime];
  NSInteger day = [dayMonthYear day];
  NSInteger month = [dayMonthYear month];
  NSInteger year = [dayMonthYear year];
	
  NSDateComponents *comps = [[NSDateComponents alloc] init];
  [comps setDay:day ];
  [comps setMonth:month];
  [comps setYear:year];
  [comps setHour:0];
  [comps setMinute:0];
  self.firstDateInTable = [gregorian dateFromComponents:comps];  // start table data at midnight local time
	
  [self.moonAz removeAllObjects];
  [self.moonEl removeAllObjects];
  [self.moonIllumPercent removeAllObjects];
  [self.sunAz removeAllObjects];
  [self.sunEl removeAllObjects];
	
  for (int i=0; i<kNumberOfDailyPoints; i++) {
  NSDate *date = [self.firstDateInTable dateByAddingTimeInterval:i*kSecondsBetweenPoints];
  // self.viewingDate = date;
  double moonAzimuth,moonElevation,illumPercent;
		
  [self calcLunarAzimuth:&moonAzimuth elevation:&moonElevation illuminatedFraction:&illumPercent apRightAscension: nil apDeclination: nil forDate:date];
		
  double sunAzimuth, sunElevation;
  [self calcSolarAzimuth:&sunAzimuth elevation: &sunElevation apRightAscension:nil apDeclination: nil forDate:date];
		
  [self.moonAz addObject: [NSNumber numberWithInt:(int) moonAzimuth]];
  [self.moonEl addObject: [NSNumber numberWithInt:(int) moonElevation]];
		
		
  [self.moonIllumPercent addObject: [NSNumber numberWithInt:(int) (100*illumPercent+0.5)]];
		
  [self.sunAz addObject: [NSNumber numberWithInt:(int) sunAzimuth]];
  [self.sunEl addObject: [NSNumber numberWithInt:(int) sunElevation]];
  }
  }

  - (void) getLunarAzimuth: (int *) azimuth elevation: (int *) elevation illuminatedFraction: (int *) illuminatedFraction forRow: (int) row{
  *azimuth = [[self.moonAz objectAtIndex:row] intValue];
  *elevation = [[self.moonEl objectAtIndex:row] intValue];
  *illuminatedFraction = [[self.moonIllumPercent objectAtIndex:row] intValue];
  }

  - (void) getSolarAzimuth: (int *)azimuth elevation: (int *) elevation forRow: (int) row{
  *azimuth = [[self.sunAz objectAtIndex:row] intValue];
  *elevation = [[self.sunEl objectAtIndex:row] intValue];
  }

  - (NSDate *) calcLunarPhaseAfterDate: (double) startingJulianDay phase: (int) whichPhase{
  //correction factors from Meeus, p. 351:  
  // interpret array rows as New Moon, Full Moon, MCoeff, MPrime Coeff, FCoeff, OmegaCoeff, EExponent
  double tableA[25][7] = {
  {-0.40720, -0.40614, 0, 1, 0, 0, 0},
  {0.17241, 0.17302, 1, 0, 0, 0, 1},
  {0.01608, 0.01614, 0, 2, 0, 0, 0},
  {0.01039, 0.01043, 0, 0, 2, 0, 0},
  {0.00739, 0.00734, -1, 1, 0, 0, 1},
  {-0.00514, -0.00515, 1, 1, 0, 0, 1},
  {0.00208, 0.00209, 2, 0, 0, 0, 2},
  {-0.00111, -0.00111, 0, 1, -2, 0, 0},
  {-0.00057, -0.00057, 0, 1, 2, 0, 0},
  {0.00056, 0.00056, 1, 2, 0, 0, 1},
  {-0.00042, -0.00042, 0, 3, 0, 0, 0},
  {0.00042, 0.00042, 1, 0, 2, 0, 1},
  {0.00038, 0.00038, 1, 0, -2, 0, 1},
  {-0.00024, -0.00024, -1, 2, 0, 0, 1},
  {-0.00017, -0.00017, 0, 0, 0, 1, 0},
  {-0.00007, -0.00007, 2, 1, 0, 0, 0},
  {0.00004, 0.00004, 0, 2, -2, 0, 0,},
  {0.00004, 0.00004, 3, 0, 0, 0, 0},
  {0.00003, 0.00003, 1, 1, -2, 0, 0},
  {0.00003, 0.00003, 0, 2, 2, 0, 0},
  {-0.00003, -0.00003, 1, 1, 2, 0, 0},
  {0.00003, 0.00003, -1, 1, 2, 0, 0},
  {-0.00002, -0.00002, -1, 1, -2, 0, 0},
  {-0.00002, -0.00002, 1, 3, 0, 0, 0},
  {0.00002, 0.00002, 0, 4, 0, 0, 0}
  };
	
  double tableB[25][6] = {
  {-0.62801, 0, 1, 0, 0, 0},
  {+0.17172, 1, 0, 0, 0, 1},
  {-0.01183, 1, 1, 0, 0, 1},
  {+0.00862, 0, 2, 0, 0, 0},
  {+0.00804, 0, 0, 2, 0, 0},
  {+0.00454, -1, 1, -0, 0, 1},
  {+0.00204, 2, 0, 0, 0, 2},
  {-0.00180, 0, 1, -2, 0, 0},
  {-0.00070, 0, 1, 2, 0, 0},
  {-0.00040, 0, 3, 0, 0, 0},
  {-0.00034, -1, 2, 0, 0, 1},
  {+0.00032, 1, 0, -2, 0, 1},
  {+0.00032, 1, 0, -2, 0, 1},
  {-0.00028, 2, 1, 0, 0, 2},
  {+0.00027, 1, 2, 0, 0, 1},
  {-0.00017, 0, 0, 0, 1, 0},
  {-0.00005, -1, 1, -2, 0, 0},
  {+0.00004, 0, 2, 2, 0, 0,},
  {-0.00004, 1, 1, 2, 0, 0},
  {+0.00004, -2, 1, 0, 0, 0},
  {+0.00003, 1, 1, -2, 0, 0},
  {+0.00003, 3, 0, 0, 0, 0},
  {+0.00002, 0, 2, -2, 0, 0},
  {+0.00002, -1, 1, 2, 0, 0},
  {-0.00002, 1, 3, 0, 0, 0}
  };
	
	
	
  // offsets in array of constants for corrections (TableA only!):
  #define	kMCoeff			2
  #define kMPrimeCoeff	3
  #define kFCoeff			4
  #define kOmegaCoeff		5
  #define kExponentOfE	6
	
	
  NSDate	*date = [NSDate dateFromJulianDay:startingJulianDay];
  NSDateComponents *dayMonthYear = [self.calendar components:(NSDayCalendarUnit | NSMonthCalendarUnit | NSYearCalendarUnit)
  fromDate:date];
  NSInteger year = [dayMonthYear year];
  // add fractional portion to year for this calculation to work properly:
  NSDateComponents *comps = [[NSDateComponents alloc] init];
  [comps setDay:1 ];
  [comps setMonth:1];
  [comps setYear:year];
  NSDate *jan1Date = [calendar dateFromComponents:comps];
  double secondsIntoYear = [date timeIntervalSinceDate:jan1Date];
  double fractionOfYear = secondsIntoYear/(365*86400);
	
  double floatYear  = year + fractionOfYear;
	
  float k = (floatYear - 2000)*12.3685;		// from Meeus.  Don't ask why.
  if (k >= 0) {
  k = floor(k+0.5);
  }
  else {
  k = ceil(k-0.5);
  }
	
  double T = k/1236.85;					// time in Julian Centuries since epoch 2000
	
  switch (whichPhase) {
  case kMoonPhaseNewMoon:
  // k is already an int. Do nothing
  break;
  case kMoonPhaseFirstQuarter:
  k = k+0.25;
  break;
  case kMoonPhaseFullMoon:
  k = k+0.5;
  break;
  case kMoonPhaseLastQuarter:
  k = k + 0.75;
  break;
  default:
  break;
  }
  // jde = 2451550.09766 + 29.530588861*k + .00015437*T^2 - 0.000000150*T^3 + .00000000073*T^4:
  double jde = T * 0.00000000073;
  jde = T*(jde - 0.000000150);
  jde = T*(jde + 0.00015437);
  jde = T*jde + 29.530588861*k + 2451550.09766;
	
	
  // *** need to add additional terms to get good results here....
	
  double E = 1 - 0.002516*T - 0.0000074*T*T;
	
  //Sun's mean anomaly:
  double M = 2.5534 + 29.10535670*k - 0.0000014*T*T - 0.00000011*T*T*T;
  M = reduce360(M);
	
  //Moon's mean anomaly:
  double MPrime = 201.5643 + 385.81693528*k + 0.0107582*T*T + 0.00001238*T*T*T - 0.000000058*T*T*T;
  MPrime = reduce360(MPrime);
	
  //Moon's argument of latitude:
  double F = 160.7108 + 390.67050284*k - 0.0016118*T*T - 0.00000227*T*T*T + .000000011*T*T*T*T;
  F = reduce360(F);
	
  //Longitude of the ascending node of the lunar orbit:
  double Omega = 124.7746 - 1.56375588*k + 0.0020672*T*T + 0.00000215*T*T*T;
  Omega = reduce360(Omega);
  double correction = 0;
	
	
  if (whichPhase == kMoonPhaseNewMoon || whichPhase == kMoonPhaseFullMoon) {
  int sinCoeffColumn = 0;	// table column to use for new moon
  if (whichPhase == kMoonPhaseFullMoon) {
  sinCoeffColumn = 1;		// column for full moon
  }
  // compute correction factors from correctionA array:
  for (int row=0; row<25; row++) {
  double sinArg = (tableA[row][kMCoeff]*M + 
  tableA[row][kMPrimeCoeff]*MPrime + 
  tableA[row][kFCoeff]*F + 
  tableA[row][kOmegaCoeff]*Omega);
  double addend = tableA[row][sinCoeffColumn]*sin(degToRad(sinArg));
  if (tableA[row][kExponentOfE] == 1.0) {
  addend = addend * E;
  }
  else if (tableA[row][kExponentOfE] == 2.0) {
  addend = addend *E*E;
  }
  correction = correction + addend;
			
  }
  }
  else {  //First quarter or last quarter:
  for (int row=0; row<25; row++) {
  double sinArg = tableB[row][1]*M + 
  tableB[row][2]*MPrime + 
  tableB[row][3]*F + 
  tableB[row][4]*Omega;
			
  double addend = tableB[row][0] * sin(degToRad(sinArg));
  if (tableB[row][5] == 1.0) {
  addend = addend*E;
  }
  else if(tableB[row][5] == 2.0){
  addend = addend*E*E;
  }
  correction = correction + addend;
			
			
  }
  // W is an incremental correction that applies only to the quarter-phases:
  double W = 0.00306 - 0.00038*E*cos(degToRad(M)) + 0.00026*cos(degToRad(MPrime)) - 0.00002*cos(degToRad(MPrime-M)) +
  0.00002*cos(degToRad(MPrime+M)) + 0.00002*cos(degToRad(2*F));
		
  if (whichPhase == kMoonPhaseFirstQuarter) {
  correction = correction + W;
  }
  else {
  correction = correction - W;
  }
		
  }
  //planetary aruments:
  double A1  = 299.77 + 0.107408*k - 0.009173*T*T;
  double A2  = 251.88 + 0.016321*k;
  double A3  = 251.83 + 26.651886*k;
  double A4  = 349.42 + 36.412478*k;
  double A5  = 84.66  + 18.206239*k;
  double A6  = 141.74 + 53.303771*k;
  double A7  = 207.14 + 2.453732*k;
  double A8  = 154.84 + 7.306860*k;
  double A9  = 34.52  + 27.261239*k;
  double A10 = 207.19 + 0.121824*k;
  double A11 = 291.34 + 1.8844379*k;
  double A12 = 161.72 + 24.198154*k;
  double A13 = 239.56 + 25.513099*k;
  double A14 = 331.55 + 3.592518*k;
	
  //calculate planetary corrections applicable to all phases:
  double planetaryInfluences = 0.000325*sin(degToRad(A1)) + 0.000165*sin(degToRad(A2)) + 0.000164*sin(degToRad(A3));
  planetaryInfluences +=	0.000126*sin(degToRad(A4)) + 0.000110*sin(degToRad(A5)) + 0.000062*sin(degToRad(A6)) + 0.000060*sin(degToRad(A7));
  planetaryInfluences +=	0.000056*sin(degToRad(A8)) + 0.000047*sin(degToRad(A9)) + 0.000042*sin(degToRad(A10)) + 0.000040*sin(degToRad(A11));
  planetaryInfluences +=  0.000037*sin(degToRad(A12)) + 0.000035*sin(degToRad(A13 )) + 0.000023*sin(degToRad(A14));
	
  jde = jde + correction + planetaryInfluences;
  NSDate *resultingDate = [NSDate dateFromJulianDay:jde];
	
	
  return resultingDate;
	
	
  }

*/

time_t calcSunOrMoonRiseForDate(time_t the_date, int oneForRiseTwoForSet, int oneForSunTwoForMoon,struct location theLocation){
  //- (NSDate *) calcSunOrMoonRiseForDate: (NSDate *) date oneForRiseTwoForSet: (NSInteger) riseOrSet
  //oneForSunTwoForMoon: (NSInteger) sunOrMoon{

  double alpha[3],delta[3],jd[3];	// right ascension and declination for the moon on day D-1,D, and D+1 at 0h dynamical time
  double h0 = 0.125;			// approximation for h0 - corrects for semidiameter of moon + refraction
  double H0;				// first-pass hour angle for rise/set relative to transit
  double m[3];			        // m0 = transit in fraction of day.  m1 = moonrise.  m2 = moonset
  double newAlpha[3];		        // second-pass right ascension calcs
  double newDelta[3];		        // second-pass declination calcs
  double H[3];			        // hour angles for transit, moonrise, moonset
  double deltaM;			// correction after interpolating new alpha, delta values
  double a,b,c;			        // interpolation variables
  time_t riseDate, setDate, transitDate;
  double myElevation, myAzimuth, myIlluminatedFraction;
	
  if (oneForSunTwoForMoon == kSun) {
    h0 = -0.83333;		        // h0 - corrects for semidiameter of sun + refraction
  }
	
  the_date = calendarDateAt0000UT(the_date,theLocation.gmt_offset);   

  jd[1] = julianDayAt0000UT(the_date);  // the day in question
  jd[0] = jd[1]-1;			// day before	
  jd[2] = jd[1]+1;			// day after (used for interpolation)
	
  double bigThetaZeroForDayD = bigThetaZeroInDegrees(dateFromJulianDay(jd[1]));
  

  // debug var:
  time_t test_time;
  test_time = dateFromJulianDay(jd[1]);
  char buf[64];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&test_time));
  #ifdef debug
  printf("confirming that midnight UT on day D = %s\n", buf);
  #endif
 
  int i;
  for (i=0; i<3; i++) {
    time_t  dateOfCalculation = dateFromJulianDay(jd[i]);
    if (oneForSunTwoForMoon == kSun) {
      calcSolarAzimuth(&myAzimuth,  &myElevation, &alpha[i], &delta[i], dateOfCalculation,theLocation);
    }
    else{
      calcLunarAzimuth(&myAzimuth,&myElevation,&myIlluminatedFraction, &alpha[i],\
		       &delta[i], dateOfCalculation,theLocation);
    }
    #ifdef debug
    printf("date: %ld, RA (hours): %f, declination: %f", dateOfCalculation, alpha[i]/15.0,delta[i]);
    #endif
  }	

  int iteration;
  for (iteration = 0; iteration<2; iteration++) {
    double cosH0 = (sin(degToRad(h0))-sin(degToRad(theLocation.latitude))*sin(degToRad(delta[1]))) / 
      (cos(degToRad(theLocation.latitude))*cos(degToRad(delta[1])));
    if (fabs(cosH0) > 1.0 ) {
      //NSLog(@"Body in question is circumpolar on this date.");
    }
    else {
      H0 = reduce360(radToDeg(acos(cosH0)));
      if (H0 > 180) {
	h0 -= 360.0;
      }
      if (iteration==0) {  // otherwise use corrected m-values from previous pass:
	//NSLog(@"iteration %i: H0 = %f",iteration,H0);
	m[0] = (alpha[1] + theLocation.longitude - bigThetaZeroForDayD)/360;  //transit as fraction of 24 hrs
	//NSLog(@"iteration %i: Transit at %f hours UT",iteration, m[0]*24);
	m[1] = m[0] - H0/360; //moonrise as fraction of 24hrs
	//NSLog(@"iteration %i: moonrise at %f hours UT",iteration, m[1]*24);
	m[2] = m[0] + H0/360; //moonset as fraction of 24 hrs
	//NSLog(@"iteration %i: moonset at %f hours UT",iteration, m[2]*24);
				
      }
			
			
      // adjust m (fraction of day) so that answers fall on the right date in local time:
			
      //double lagRelativeToGMT = -(double) [theLocation.timeZone secondsFromGMTForDate:date]/86400; //lag as fraction of a day
      double lagRelativeToGMT = -(double) (theLocation.gmt_offset*3600) / 86400.0;
      
  for (i=0; i<3; i++) {
	if (m[i] < lagRelativeToGMT) {
	  m[i] += 1.0;
	}
	if (m[i] > 1+lagRelativeToGMT) {
	  m[i] -= 1.0;
	}
      }

      //approximate rise:
      double riseJD = julianDayAt0000UT(the_date) +  m[1];
      riseDate = dateFromJulianDay(riseJD);
      //NSLog(@"Before Correction: moonrise = %@",moonriseDate);
			
      //approximate set:
      double setJD = julianDayAt0000UT(the_date) +  m[2];
      setDate = dateFromJulianDay(setJD);
      //NSLog(@"Before Correction: %@",moonsetDate);

      //now start correcting the m-values:
      for (i=0; i<3; i++) {
	double theta0 = bigThetaZeroForDayD + 360.985647*m[i]; 
	theta0 = reduce360(theta0);
	double n = m[i] + 75.0/86400;  // replace with real delta T calculation later
				
	//interpolate a new alpha:
	a = reduce360(alpha[1]-alpha[0]);	// reduce360 takes care of "wraparound" cases
	b = reduce360(alpha[2]-alpha[1]) ;
	c = b-a;
	newAlpha[i] = alpha[1] + (n/2)*(a+b+n*c);
	//NSLog(@"interpolated alpha = %f",newAlpha[i]);
				
	a = delta[1]-delta[0];
	b = delta[2]-delta[1];
	c = b-a;
	newDelta[i] = delta[1] + (n/2)*(a+b+n*c);
	//NSLog(@"interpolated delta = %f",newDelta[i]);
				
	H[i] = theta0 - theLocation.longitude - newAlpha[i];  // hour angle refined with new alpha
				
	double sinH = sin(degToRad(theLocation.latitude))*sin(degToRad(newDelta[i])) + 
	  cos(degToRad(theLocation.latitude))*cos(degToRad(newDelta[i]))*cos(degToRad(H[i]));
	double h = radToDeg(asin(sinH));		// elevation refined with new delta, m.
				
	if (i==0) {
	  deltaM = -H[i]/360.0;  // correction for transit
	}
	else {
	  deltaM = (h-h0)/(360.0*cos(degToRad(newDelta[i]))*cos(degToRad(theLocation.latitude))*sin(degToRad(H[i])));
					
	}
				
	//NSLog(@"delta M[%i] = %f", i, deltaM);
	m[i] = m[i] + deltaM;	// apply correction
				
      }
			
			
      //approximate rise:
      riseJD = julianDayAt0000UT(the_date) +  m[1];
      riseDate = dateFromJulianDay(riseJD);


						
      //approximate mset:
      setJD = julianDayAt0000UT(the_date) +  m[2];
      setDate = dateFromJulianDay(setJD);
						
    }
		
  }
  if (oneForRiseTwoForSet == 1) {
    return riseDate;
  }
  else{
    return setDate;
  }
	
	
}
/*
int main(){
  struct location myLocation;
  double myAzimuth,myElevation,myFullness,myRA,myDeclination;
  time_t now,sunset,sunrise;
  char riseOrSet[2][16] = {"rise","set"};
  char sunOrMoon[2][16] = {"Sun","Moon"};
  char buf[64];

  init_location(&myLocation);
  
  now = time(NULL);
  // now = 1498401720;
  
  calcLunarAzimuth(&myAzimuth,&myElevation,&myFullness,&myRA,&myDeclination,now,myLocation);
  printf("\n");
  printf("%s\n",ctime(&now));
  printf("%f long, %f lat\n",myLocation.longitude,myLocation.latitude);
  printf("Moon azimuth: %f elevation: %f\n",myAzimuth, myElevation);
  printf("\n");

  calcSolarAzimuth(&myAzimuth,&myElevation,&myRA,&myDeclination,now,myLocation);
  printf("\n");
  printf("%s\n",ctime(&now));
  printf("Sun azimuth: %f elevation: %f\n",myAzimuth, myElevation);
  printf("\n");

  int som;
  for(som=1;som<3;som++)   
    for(int ros=1;ros<3;ros++){
      sunset = calcSunOrMoonRiseForDate(time(NULL)+86400, ros,som, myLocation);
      //sunset = calcSunOrMoonRiseForDate(1513453227, ros,som, myLocation);
      sunset += 3600*myLocation.gmt_offset;
      strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S\n", localtime(&sunset));
      printf("%s%s: %s",sunOrMoon[som-1],riseOrSet[ros-1],buf);
    }
  printf("\n");
  printf("Moon is %f percent full\n\n",100*myFullness);

  now = time(NULL);
  time_t mitz = midnightInTimeZone(now, myLocation.gmt_offset);
  printf("Midnight in time zone is %s", ctime(&mitz));
}
*/