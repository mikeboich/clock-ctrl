/*******************************************************************************
* File Name: one_pps_int.h
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
#if !defined(CY_ISR_one_pps_int_H)
#define CY_ISR_one_pps_int_H


#include <cytypes.h>
#include <cyfitter.h>

/* Interrupt Controller API. */
void one_pps_int_Start(void);
void one_pps_int_StartEx(cyisraddress address);
void one_pps_int_Stop(void);

CY_ISR_PROTO(one_pps_int_Interrupt);

void one_pps_int_SetVector(cyisraddress address);
cyisraddress one_pps_int_GetVector(void);

void one_pps_int_SetPriority(uint8 priority);
uint8 one_pps_int_GetPriority(void);

void one_pps_int_Enable(void);
uint8 one_pps_int_GetState(void);
void one_pps_int_Disable(void);

void one_pps_int_SetPending(void);
void one_pps_int_ClearPending(void);


/* Interrupt Controller Constants */

/* Address of the INTC.VECT[x] register that contains the Address of the one_pps_int ISR. */
#define one_pps_int_INTC_VECTOR            ((reg32 *) one_pps_int__INTC_VECT)

/* Address of the one_pps_int ISR priority. */
#define one_pps_int_INTC_PRIOR             ((reg8 *) one_pps_int__INTC_PRIOR_REG)

/* Priority of the one_pps_int interrupt. */
#define one_pps_int_INTC_PRIOR_NUMBER      one_pps_int__INTC_PRIOR_NUM

/* Address of the INTC.SET_EN[x] byte to bit enable one_pps_int interrupt. */
#define one_pps_int_INTC_SET_EN            ((reg32 *) one_pps_int__INTC_SET_EN_REG)

/* Address of the INTC.CLR_EN[x] register to bit clear the one_pps_int interrupt. */
#define one_pps_int_INTC_CLR_EN            ((reg32 *) one_pps_int__INTC_CLR_EN_REG)

/* Address of the INTC.SET_PD[x] register to set the one_pps_int interrupt state to pending. */
#define one_pps_int_INTC_SET_PD            ((reg32 *) one_pps_int__INTC_SET_PD_REG)

/* Address of the INTC.CLR_PD[x] register to clear the one_pps_int interrupt. */
#define one_pps_int_INTC_CLR_PD            ((reg32 *) one_pps_int__INTC_CLR_PD_REG)


#endif /* CY_ISR_one_pps_int_H */


/* [] END OF FILE */
