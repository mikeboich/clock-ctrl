/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

/* [] END OF FILE */

#include "time.h"
#include "ViewingLocation.h"
time_t calcSunOrMoonRiseForDate(time_t the_date, int oneForRiseTwoForSet, int oneForSunTwoForMoon,struct location theLocation);
void calcLunarAzimuth(double *azimuthResult, double *elevationResult, double *fullness, double *rightAscensionResult, double *declinationResult, time_t theDate,struct location theLocation);
void calcSolarAzimuth(double *azimuthResult, double *elevationResult, double *rightAscensionResult, double *declinationResult, time_t theDate, struct location theLocation);