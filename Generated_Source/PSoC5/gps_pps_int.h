/*******************************************************************************
* File Name: gps_pps_int.h
* Version 1.70
*
*  Description:
*   Provides the function definitions for the Interrupt Controller.
*
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/
#if !defined(CY_ISR_gps_pps_int_H)
#define CY_ISR_gps_pps_int_H


#include <cytypes.h>
#include <cyfitter.h>

/* Interrupt Controller API. */
void gps_pps_int_Start(void);
void gps_pps_int_StartEx(cyisraddress address);
void gps_pps_int_Stop(void);

CY_ISR_PROTO(gps_pps_int_Interrupt);

void gps_pps_int_SetVector(cyisraddress address);
cyisraddress gps_pps_int_GetVector(void);

void gps_pps_int_SetPriority(uint8 priority);
uint8 gps_pps_int_GetPriority(void);

void gps_pps_int_Enable(void);
uint8 gps_pps_int_GetState(void);
void gps_pps_int_Disable(void);

void gps_pps_int_SetPending(void);
void gps_pps_int_ClearPending(void);


/* Interrupt Controller Constants */

/* Address of the INTC.VECT[x] register that contains the Address of the gps_pps_int ISR. */
#define gps_pps_int_INTC_VECTOR            ((reg32 *) gps_pps_int__INTC_VECT)

/* Address of the gps_pps_int ISR priority. */
#define gps_pps_int_INTC_PRIOR             ((reg8 *) gps_pps_int__INTC_PRIOR_REG)

/* Priority of the gps_pps_int interrupt. */
#define gps_pps_int_INTC_PRIOR_NUMBER      gps_pps_int__INTC_PRIOR_NUM

/* Address of the INTC.SET_EN[x] byte to bit enable gps_pps_int interrupt. */
#define gps_pps_int_INTC_SET_EN            ((reg32 *) gps_pps_int__INTC_SET_EN_REG)

/* Address of the INTC.CLR_EN[x] register to bit clear the gps_pps_int interrupt. */
#define gps_pps_int_INTC_CLR_EN            ((reg32 *) gps_pps_int__INTC_CLR_EN_REG)

/* Address of the INTC.SET_PD[x] register to set the gps_pps_int interrupt state to pending. */
#define gps_pps_int_INTC_SET_PD            ((reg32 *) gps_pps_int__INTC_SET_PD_REG)

/* Address of the INTC.CLR_PD[x] register to clear the gps_pps_int interrupt. */
#define gps_pps_int_INTC_CLR_PD            ((reg32 *) gps_pps_int__INTC_CLR_PD_REG)


#endif /* CY_ISR_gps_pps_int_H */


/* [] END OF FILE */
